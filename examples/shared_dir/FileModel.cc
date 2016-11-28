#include "FileModel.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>

namespace {

int creatDirAndReturnFd(const char* path) {
  int ret = open(path, O_RDONLY | O_DIRECTORY, 0700);
  if (ret != -1) {
    return ret;
  } else {
    unlink(path);
    mkdir(path, 0700);
    return open(path, O_RDONLY | O_DIRECTORY, 0700);
  }
}

int creatDirAndReturnFdAt(int fd, const char* path) {
  int ret = openat(fd, path, O_RDONLY | O_DIRECTORY, 0700);
  if (ret != -1) {
    return ret;
  } else {
    unlinkat(fd, path, 0);
    mkdirat(fd, path, 0700);
    return openat(fd, path, O_RDONLY | O_DIRECTORY, 0700);
  }
}

std::string getName(const std::string& seed) {
  std::ostringstream os;
  os << seed << Timestamp::now().toFormattedString();
  return os.str();
}

}

FileModel::FileModel(const std::string path) {
  _workDirPath = path;
  _workDirFd = creatDirAndReturnFd(path.c_str());
  _tempDirFd = creatDirAndReturnFdAt(_workDirFd, "temp");
}

std::string FileModel::creatTempFileForReceive(const std::string& seed) {
  auto name = getUniqueName(seed, _tempDirFd);
  int ret = openat(_tempDirFd, name.c_str(), O_RDONLY | O_CREAT, 0700);
  if (ret == -1) {
    LOG_ERROR << "FileModel::creatTempFileForReceive";
  }
  close(ret);
  return name;
}

std::string FileModel::creatTempFileForSend(const std::string& seed, const std::string& fileName) {
  auto name = getUniqueName(seed, _tempDirFd);
  int ret = linkat(_workDirFd, fileName.c_str(), _tempDirFd, name.c_str(), 0);
  if (ret == -1) {
    LOG_ERROR << "FileModel::creatTempFileForSend";
  }
  return name;
}

FileModel::FileMap FileModel::scanfPath(const std::string& path) {
  FileMap result;
  DIR* dirPtr = opendir(path.c_str());
  int dirFd = dirfd(dirPtr);
  struct dirent *dp;
  if (dirPtr == nullptr) {
    LOG_ERROR << "FileModel::scanfPath - no accessed dir: " << path;
  }
  while ((dp = readdir(dirPtr)) != nullptr) {
    int fd = openat(dirFd, dp->d_name, O_RDONLY, 0700);
    struct stat temp;
    int ret = fstat(fd, &temp);
    if (ret == -1) {
      LOG_ERROR << "FileModel::scanfPath - fstat: " << errno;
    }
    if (S_ISREG(temp.st_mode) && std::string(dp->d_name) != ".DS_Store") {
      char buf[64];
      sprintf(buf, "%lu", temp.st_mtime);
      result.emplace(dp->d_name, std::string(buf));
    }
  }
  return result;
}

FileModel::FileMap FileModel::scanfPath(const int dirFd) {
  FileMap result;
  DIR* dirPtr = fdopendir(dirFd);
  struct dirent *dp;
  if (dirPtr == nullptr) {
    LOG_ERROR << "FileModel::scanfPath - no accessed dir: " << dirFd;
  }
  while ((dp = readdir(dirPtr)) != nullptr) {
    int fd = openat(dirFd, dp->d_name, O_RDONLY, 0700);
    struct stat temp;
    int ret = fstat(fd, &temp);
    if (ret == -1) {
      LOG_ERROR << "FileModel::scanfPath - fstat: " << errno;
    }
    if (S_ISREG(temp.st_mode) && std::string(dp->d_name) != ".DS_Store") {
      char buf[64];
      sprintf(buf, "%lu", temp.st_mtime);
      result.emplace(dp->d_name, std::string(buf));
    }
  }
  return result;
}

std::string FileModel::fileMapToString(const FileModel::FileMap& fileMap) {
  std::stringstream ss;
  for (auto it = fileMap.begin(); it != fileMap.end(); it++) {
    ss << it->first << "\n" << it->second << "\n";
  }
  return ss.str();
}

FileModel::FileMap FileModel::stringToFileMap(const std::string& str) {
  FileMap result;
  std::istringstream is(str);
  std::string ls, rs;
  while (is >> ls) {
    is >> rs;
    result.emplace(ls, rs);
  }
  return result;
}

FileModel::CmpReturn FileModel::fileMapCmper(const FileMap& client, const FileMap& server) {
  FileNameList loadToClient, loadToServer;
  for (auto& it : server) {
    auto i = client.find(it.first);
    if (i == client.end() || it.second > i->second) {
      loadToClient.push_back(it.first);
    }
  }
  for (auto& it : client) {
    auto i = server.find(it.first);
    if (i == server.end() || it.second > i->second) {
      loadToServer.push_back(it.first);
    }
  }
  return CmpReturn(loadToClient, loadToServer);
}

std::string FileModel::getUniqueName(const std::string &seed, int dirFd) {
  auto files = scanfPath(dirFd);
  auto name = getName(seed);
  while (files.find(name) != files.end()) {
    auto name = getName(seed);
  }
  return name;
}

void FileModelServer::readConfigFile() {
  std::ifstream file;
  file.open(_workDirPath + "/config");
  if (file.bad()) {
    LOG_ERROR << "open config failed";
  }
  std::string name, password;
  while (file >> name) {
    file >> password;
    _usersList.emplace(name, password);
  }
  file.close();
}

void FileModelClient::readConfigFile() {
  std::ifstream file;
  file.open(_workDirPath + "/config");
  if (!file.good()) {
    LOG_ERROR << "open config failed, run fileSyncConfig first";
  }
  std::string temp;
  if (file >> temp) {
    _sharedDirPath = temp;
  } else {
    LOG_ERROR << "cannot find shared dir path, run fileSyncConfig first";
  }
  if (file >> temp) {
    _username = temp;
  } else {
    LOG_ERROR << "cannot find username, run fileSyncConfig first";
  }
  if (file >> temp) {
    _password = temp;
  } else {
    LOG_ERROR << "cannot find password, run fileSyncConfig first";
  }
  file.close();
}

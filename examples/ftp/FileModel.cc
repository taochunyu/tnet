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

int createDirAndReturnFd(const char* path) {
  int ret = open(path, O_RDONLY | O_DIRECTORY, 0700);
  if (ret != -1) {
    return ret;
  } else {
    unlink(path);
    mkdir(path, 0700);
    return open(path, O_RDONLY | O_DIRECTORY, 0700);
  }
}

int createDirAndReturnFdAt(int fd, const char* path) {
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
  os << seed << ":" << Timestamp::now().microSecondsSinceEpoch();
  return os.str();
}

}

FileModel::FileModel(const std::string path) {
  _workDirPath = path;
  _workDirFd = createDirAndReturnFd(path.c_str());
  _tempDirFd = createDirAndReturnFdAt(_workDirFd, "temp");
}

std::string FileModel::createTempFileForReceive(const std::string& seed) {
  auto name = getUniqueName(seed, _tempDirFd);
  int ret = openat(_tempDirFd, name.c_str(), O_RDONLY | O_CREAT, 0700);
  if (ret == -1) {
    LOG_ERROR << "FileModel::createTempFileForReceive";
  }
  close(ret);
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

std::string FileModelServer::createTempFileForSend(const std::string& seed, const std::string& fileName) {
  auto name = getUniqueName(seed, _tempDirFd);
  int ret = linkat(_sharedFd, fileName.c_str(), _tempDirFd, name.c_str(), AT_SYMLINK_FOLLOW);
  if (ret == -1) {
    LOG_ERROR << "FileModel::createTempFileForSend";
  }
  return name;
}

void FileModelServer::lockLink(const std::string& from, const std::string& to) {
  MutexLockGuard lck(_mtx);
  struct stat fromStat, toStat;
  int ret = fstatat(_tempDirFd, from.c_str(), &fromStat, 0);
  if (ret == -1) {
    LOG_SYSERR << "FileModel::lockLink";
  }
  std::string toPath = _sharedDirPath + to;
  ret = stat(toPath.c_str(), &toStat);
  if (ret == -1) {
    LOG_SYSERR << "FileModel::lockLink";
  }
  if (fromStat.st_atime > toStat.st_atime) {
    unlink(toPath.c_str());
    linkat(_tempDirFd, from.c_str(), _sharedFd, to.c_str(), AT_SYMLINK_FOLLOW);
  } else {
    unlinkat(_tempDirFd, from.c_str(), 0);
  }
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
  _sharedFd = open(_sharedDirPath.c_str(), O_RDONLY, 0700);
}

std::string FileModelClient::createTempFileForSend(const std::string& seed, const std::string& fileName) {
  auto name = getUniqueName(seed, _tempDirFd);
  int ret = linkat(_sharedFd, fileName.c_str(), _tempDirFd, name.c_str(), AT_SYMLINK_FOLLOW);
  printf("没阻塞\n");
  if (ret == -1) {
    LOG_ERROR << "FileModel::createTempFileForSend";
  }
  return name;
}

void FileModelClient::lockLink(const std::string& from, const std::string& to) {
  MutexLockGuard lck(_mtx);
  struct stat fromStat, toStat;
  int ret = fstatat(_tempDirFd, from.c_str(), &fromStat, AT_SYMLINK_NOFOLLOW);
  if (ret == -1) {
    LOG_SYSERR << "FileModel::lockLink " << strerror(errno) << " " << from;
  }
  std::string toPath = _sharedDirPath + "/" + to;
  ret = stat(toPath.c_str(), &toStat);
  printf("%s\n", toPath.c_str());
  if (ret == -1) {
    LOG_SYSERR << "FileModel::lockLink";
  }
  if (fromStat.st_atime > toStat.st_atime) {
    printf("删除\n");
    //unlink(toPath.c_str());
    linkat(_tempDirFd, from.c_str(), _sharedFd, to.c_str(), AT_SYMLINK_FOLLOW);
  } else {
    printf("不删除\n");
    unlinkat(_tempDirFd, from.c_str(), 0);
  }
}

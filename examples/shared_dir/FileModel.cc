#include "FileModel.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sstream>

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

}

FileModel::FileModel(const std::string path) {
  _workDirFd = creatDirAndReturnFd(path.c_str());
  _tempDirFd = creatDirAndReturnFdAt(_workDirFd, "temp");
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
    if (S_ISDIR(temp.st_mode)) {
      LOG_ERROR << "dir " << dp->d_name << " in shared dir";
    }
    char buf[64];
    sprintf(buf, "%lu", temp.st_mtime);
    result.emplace(dp->d_name, std::string(buf));
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

FileModel::CmpReturn FileModel::FileMapCmper(const FileMap& client, const FileMap& server) {
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

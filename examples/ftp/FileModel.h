#ifndef FILEMODEL_H
#define FILEMODEL_H

#ifndef _DARWIN_FEATURE_64_BIT_INODE
#define _DARWIN_FEATURE_64_BIT_INODE
#endif  // _DARWIN_FEATURE_64_BIT_INODE

#include "tnet.h"
#include <map>
#include <string>
#include <fcntl.h>

class FileModel : tnet::nocopyable {
 public:
  using FileMap = std::map<std::string, std::string>;
  using FileNameList = std::vector<std::string>;
  using CmpReturn = std::pair<FileNameList, FileNameList>;

  FileModel(const std::string path = "/tmp/fileSync");
  void lockLink(const std::string& from, const std::string& to, const std::string& create);

  static FileMap scanfPath(const std::string& path);
  static FileMap scanfPath(const int dirFd);
  static std::string fileMapToString(const FileMap&);
  static FileMap stringToFileMap(const std::string& str);
  static CmpReturn fileMapCmper(const FileMap&, const FileMap&);
  static std::string getUniqueName(const std::string& seed, int dirFd);
 protected:
  int _workDirFd;
  int _tempDirFd;
  std::string _workDirPath;
  std::string _sharedDirPath;
  int         _sharedFd;
  MutexLock _mtx;
};

class FileModelServer : public FileModel {
  friend class MessageServer;
  friend class FileServer;
 public:
  FileModelServer() : FileModel("/tmp/fileSyncServer") {
     _sharedDirPath = "/Users/taochunyu/Desktop/server";
    _sharedFd = open(_sharedDirPath.c_str(), O_RDONLY, 0700);
  }
  void readConfigFile();
  std::string createTempFileForReceive(const std::string& seed, const std::string& name);
  std::string createTempFileForSend(const std::string& seed, const std::string& fileName);
  void addUser(std::string, std::string);
 private:
  std::map<std::string, std::string> _usersList;
  MutexLock                          _usersMtx;
};

class FileModelClient : public FileModel {
  friend class MessageClient;
  friend class FileClient;
 public:
  FileModelClient() : FileModel() {}
  void readConfigFile();
  std::string createTempFileForReceive(const std::string& seed);
  std::string createTempFileForSend(const std::string& seed, const std::string& fileName);
 private:
  std::string _username;
  std::string _password;
};

#endif  // FILEMODEL_H

#ifndef FILEMODEL_H
#define FILEMODEL_H

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
  virtual void readConfigFile() = 0;
  std::string createTempFileForReceive(const std::string& seed);
  virtual std::string createTempFileForSend(const std::string& seed, const std::string& fileName) = 0;
  virtual void lockLink(const std::string& from, const std::string& to) = 0;

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
  MutexLock _mtx;
};

class FileModelServer : public FileModel {
  friend class MessageServer;
  friend class FileServer;
 public:
  FileModelServer() : FileModel("/tmp/fileSyncServer") {
    _sharedFd = open(_sharedDirPath.c_str(), O_RDONLY, 0700);
  }
  virtual void readConfigFile();
  virtual void lockLink(const std::string& from, const std::string& to);
  std::string createTempFileForSend(const std::string& seed, const std::string& fileName);
 private:
  std::map<std::string, std::string> _usersList;
  std::string                        _sharedDirPath = "/Users/taochunyu/Desktop/server";
  int                                _sharedFd;
};

class FileModelClient : public FileModel {
  friend class MessageClient;
  friend class FileClient;
 public:
  FileModelClient() : FileModel() {}
  virtual void readConfigFile();
  virtual void lockLink(const std::string& from, const std::string& to);
  std::string createTempFileForSend(const std::string& seed, const std::string& fileName);
 private:
  std::string _sharedDirPath;
  int         _sharedFd;
  std::string _username;
  std::string _password;
};

#endif  // FILEMODEL_H

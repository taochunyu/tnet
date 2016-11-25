#ifndef FILEMODEL_H
#define FILEMODEL_H

#include "tnet.h"
#include <map>
#include <string>

class FileModel : tnet::nocopyable {
 public:
  using FileMap = std::map<std::string, std::string>;
  using FileNameList = std::vector<std::string>;
  using CmpReturn = std::pair<FileNameList, FileNameList>;

  FileModel(const std::string path = "/tmp/fileSync");
  virtual void readConfigFile() = 0;
  virtual std::string creatTempFile() = 0;

  static FileMap scanfPath(const std::string& path);
  static std::string fileMapToString(const FileMap&);
  static FileMap stringToFileMap(const std::string& str);
  static CmpReturn fileMapCmper(const FileMap&, const FileMap&);
  static std::string getUniqueName(const std::string& seed, std::string& path);
 protected:
  int _workDirFd;
  int _tempDirFd;
  std::string _workDirPath;
};

class FileModelServer : public FileModel {
  friend class MessageServer;
  friend class FileServer;
 public:
  FileModelServer() : FileModel("/tmp/fileSyncServer") {}
  virtual void readConfigFile();
  virtual std::string creatTempFile();
 private:
  std::map<std::string, std::string> _usersList;
  std::string                        _sharedDirPath = "/Users/taochunyu/Desktop/server";
};

class FileModelClient : public FileModel {
  friend class MessageClient;
 public:
  FileModelClient() : FileModel() {}
  virtual void readConfigFile();
  virtual std::string creatTempFile();
 private:
  std::string _sharedDirPath;
  std::string _username;
  std::string _password;
};

#endif  // FILEMODEL_H

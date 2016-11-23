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

  static FileMap scanfPath(const std::string& path);
  static std::string fileMapToString(const FileMap&);
  static FileMap stringToFileMap(const std::string& str);
  static CmpReturn FileMapCmper(const FileMap&, const FileMap&);
 private:
  int _workDirFd;
  int _tempDirFd;
};

#endif  // FILEMODEL_H

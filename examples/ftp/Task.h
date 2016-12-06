#ifndef TASK_H
#define TASK_H

#include "tnet.h"
#include <sstream>

struct Task {
  Task() {};
  Task(std::string a, std::string f, std::string t, std::string n, std::string s, std::string c)
      : action(a), from(f), to(t), name(n), size(s), create(c) {}
  bool operator==(const Task& rhs) const { return from == rhs.from; }
  bool operator<(const Task& rhs) const { return from < rhs.from; }
  bool operator>(const Task& rhs) const { return from > rhs.from; }
  std::string action;  // loadToServer or loadToClient
  std::string from;    // file name in temp dir
  std::string to;      // file name in temp dir
  std::string name;    // file name in shared dir
  std::string size;    // file size
  std::string create;  // microseconds since Epoch
};

std::pair<std::string, Task> decodeIpTask(std::string& message);

std::string encodeIpTask(std::string& ipPort, Task& task);

#endif  // TASK_H

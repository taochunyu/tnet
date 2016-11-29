#ifndef TASK_H
#define TASK_H

#include "tnet.h"

struct Task {
  Task() {};
  Task(std::string a, std::string f, std::string t, std::string n)
      : action(a), from(f), to(t), name(n) {}
  bool operator==(const Task& rhs) const { return from == rhs.from; }
  bool operator<(const Task& rhs) const { return from < rhs.from; }
  bool operator>(const Task& rhs) const { return from > rhs.from; }
  std::string action;  // loadToServer or loadToClient
  std::string from;    // file name in temp dir
  std::string to;      // file name in temp dir
  std::string name;    // file name in shared dir
};


#endif  // TASK_H

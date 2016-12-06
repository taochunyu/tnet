#include "Task.h"

std::pair<std::string, Task> decodeIpTask(std::string& message) {
  std::istringstream is(message);
  std::string ipPort, action, from, to, name, size, create;
  is >> ipPort >> action >> from >> to >> name >> size >> create;
  Task task(action, from, to, name, size, create);
  return std::make_pair(ipPort, task);
}

std::string encodeIpTask(std::string& ipPort, Task& task) {
  std::ostringstream os;
  os
    << ipPort << "\n" << task.action << "\n"
    << task.from << "\n" << task.to << "\n"
    << task.name << "\n" << task.size << "\n"
    << task.create;
  return os.str();
}

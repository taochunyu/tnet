#ifndef WORKERMANAGER_H
#define WORKERMANAGER_H

#include "tnet.h"
#include "Task.h"
#include "FileModel.h"
#include "FileClient.h"

class WorkerManager : tnet::nocopyable {
 public:
  using Worker = std::shared_ptr<FileClient>;
  explicit WorkerManager(InetAddress fileServer, FileModelClient& fmc, size_t numWorker = 1) {
    for (size_t i = 0; i < numWorker; i++) {
      EventLoopThread loopThread;
      auto worker = std::make_shared<FileClient>(loopThread.startLoop(), fileServer, fmc);
      worker->connect();
      _workers.push_back(worker);
    }
  }
  std::vector<Worker> callTogether() {
    MutexLockGuard lck(_mtx);
    std::vector<Worker> ret;
    for (size_t i = 0; i < _workers.size(); i++) {
      if (_workers[i]->connected()) {
        ret.push_back(_workers[i]);
      } else {
        _unhealthyWorkersId.push_back(i);
      }
    }
    return ret;
  }
  void askWorkerToDo(Worker worker, Task task) {
    MutexLockGuard lck(_mtx);
    _taskWorkerMap.emplace(task, worker);
    worker->newTask(task);
  }
  Worker whoOwnThisTask(Task task) {
    MutexLockGuard lck(_mtx);
    auto it = _taskWorkerMap.find(task);
    assert( it != _taskWorkerMap.end());
    return it->second;
  }
  std::vector<Worker> checkWorkers() {
    MutexLockGuard lck(_mtx);
    std::vector<Worker> ret;
    if (_unhealthyWorkersId.size() != 0) {
      auto it = _unhealthyWorkersId.begin();
      while (it != _unhealthyWorkersId.end()) {
        if (_workers[*it]->connected()) {
          ret.push_back(_workers[*it]);
          it = _unhealthyWorkersId.erase(it);
        }
      }
    }
    return ret;
  }
 private:
  std::map<Task, Worker> _taskWorkerMap;
  std::vector<Worker>    _workers;
  std::vector<int>       _unhealthyWorkersId;
  MutexLock              _mtx;
};

#endif  // WORKMANAGER_H

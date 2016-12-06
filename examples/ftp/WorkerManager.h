#ifndef WORKERMANAGER_H
#define WORKERMANAGER_H

#include "tnet.h"
#include "Task.h"
#include "FileModel.h"
#include "FileClient.h"

class WorkerManager : tnet::nocopyable {
  friend class MessageClient;
 public:
  using Worker = std::shared_ptr<FileClient>;
  using Sender = std::function<void(std::string, std::string)>;
  explicit WorkerManager(EventLoop* loop, InetAddress fileServer, FileModelClient& fmc, size_t numWorker = 1) : _loop(loop), _fmc(fmc) {
    for (size_t i = 0; i < numWorker; i++) {
      auto loopThread = std::make_unique<EventLoopThread>();
      auto worker = std::make_shared<FileClient>(loopThread->startLoop(), fileServer, fmc, whenWorkerFinishedTask());
      worker->connect();
      _workers.push_back(worker);
      _loopThreads.push_back(std::move(loopThread));
    }
  }

  void setSender(Sender send) {
    _send = send;
  }

  void doThoseTasks(std::vector<Task> tasks) {
    {
      MutexLockGuard lck(_taskMtx);
      _tasks = tasks;
    }
    setAll(_tasks.size());
    printf("设置总数为：%d\n", _all);
    auto allWorkers = callTogether();
    for (auto& it : allWorkers) {
      if (_tasks.size() == 0) break;
      auto task = getOneTask();
      auto ipPort = askWorkerToDo(it, task);
      auto mess = encodeIpTask(ipPort, task);
      printf("检查%s %s %s %s %s %s\n", task.action.c_str(), task.from.c_str(), task.to.c_str(), task.name.c_str(), task.size.c_str(), task.create.c_str());
      _send("/newTask", mess);
    }
  }
  void ready(Task task, Callback cb) {
    auto work = whoOwnThisTask(task);
    work->sendFile(cb);
  }
  void nextTask(Task task) {
    finishOne();
    if (_tasks.size() == 0) {
      printf("finish all tasks\n");
      return;
    }
    auto worker = whoOwnThisTask(task);
    auto next = getOneTask();
    auto ipPort = askWorkerToDo(worker, next);
    auto mess = encodeIpTask(ipPort, next);
    _send("/newTask", mess);
  }
  void setAll(int num) {
    MutexLockGuard lck(_finMtx);
    _all = num;
    _finished = 0;
  }
  void finishOne() {
    MutexLockGuard lck(_finMtx);
    ++_finished;
    if (_finished == _all) {
      printf("Finish all tasks\n");
      exit(0);
    }
  }
 private:
  Task getOneTask() {
    MutexLockGuard lck(_taskMtx);
    assert(_tasks.size() != 0);
    auto temp = _tasks.back();
    _tasks.pop_back();
    return temp;
  }
  std::function<bool(Task)> whenWorkerFinishedTask() {
    return [this](auto task)->bool{
      finishOne();
      auto worker = whoOwnThisTask(task);
      printf("我执行了\n");
      if (task.action == "loadToClient") {
        close(worker->_currentTaskFd);
        _fmc.lockLink(worker->_currentTask.to, worker->_currentTask.name);
      }
      if (_tasks.size() == 0) {
        LOG_INFO << "Finish all tasks";
        return false;
      }
      auto next = getOneTask();
      auto ipPort = askWorkerToDo(worker, next);
      auto mess = encodeIpTask(ipPort, next);
      _send("/newTask", mess);
      return true;
    };
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
    printf("工作人员：%lu\n", ret.size());
    return ret;
  }
  std::string askWorkerToDo(Worker worker, Task task) {
    MutexLockGuard lck(_mtx);
    printf("添加\n");
    _taskWorkerMap.emplace(task, worker);
    return worker->newTask(task);
  }
  Worker whoOwnThisTask(Task task) {
    MutexLockGuard lck(_mtx);
    auto it = _taskWorkerMap.find(task);
    printf("%lu\n", _taskWorkerMap.size());
    auto ii = _taskWorkerMap.begin()->first;
    printf("任务%s %s %s\n", task.action.c_str(), task.to.c_str(), task.name.c_str());
    printf("存的任务%s %s %s\n", ii.action.c_str(), ii.to.c_str(), ii.name.c_str());
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

  EventLoop*             _loop;
  std::map<Task, Worker> _taskWorkerMap;
  std::vector<Worker>    _workers;
  std::vector<int>       _unhealthyWorkersId;
  MutexLock              _mtx;

  std::vector<Task>      _tasks;
  MutexLock              _taskMtx;

  int                    _all;
  int                    _finished;
  MutexLock              _finMtx;

  Sender                 _send;
  FileModelClient&       _fmc;
  std::vector<std::unique_ptr<EventLoopThread>> _loopThreads;
};

#endif  // WORKMANAGER_H

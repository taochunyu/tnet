#ifndef MESSAGECLIENT_H
#define MESSAGECLIENT_H

#include "tnet.h"
#include "FileModel.h"
#include "FileClient.h"

class MessageClient : tnet::nocopyable {
 public:
  using Job = FileClient::Job;
  MessageClient(EventLoop* loop, InetAddress serverAddr, FileModelClient& fmc, FileClient& fc);
  void connect() { _client.connect(); }
  void send(std::string method, std::string message);
  void newJob(Job job) { _jobs.push_back(job); }
 private:
  void login(const std::string& username, const std::string& password);

  void handleConn(const TcpConnectionPtr& conn);
  void login(Ctx ctx);
  void logup(Ctx ctx);
  void jobs(Ctx ctx);

  void doJobs();

  EventLoop*       _loop;
  TcpClient        _client;
  Dispather        _dispather;
  Codec            _codec;
  TcpConnectionPtr _conn;
  std::string      _username;
  std::string      _password;
  MutexLock        _mtx;
  FileModelClient& _fmc;
  FileClient&      _fc;
  std::vector<Job> _jobs;
};

#endif  // MESSAGECLIENT_H

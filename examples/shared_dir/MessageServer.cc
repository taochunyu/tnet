#include "MessageServer.h"

MessageServer::MessageServer(EventLoop* loop, InetAddress listenAddr, FileModel& fm)
    : _loop(loop),
      _server(loop, listenAddr, "MessageServer"),
      _dispather("MessageDispather"),
      _codec(_dispather.watcher()),
      _fm(fm)
{
  _server.onConnection([this](auto conn){ handleConn(conn); });
  _server.onMessage([this](auto conn, auto buf, auto receiveTime) {
    _codec.onMessage(conn, buf, receiveTime);
  });
  _dispather.route("/logup", [this](auto ctx) { logup(ctx); });
  _dispather.configDone();
}

void MessageServer::handleConn(const TcpConnectionPtr &conn) {
  LOG_INFO
    << conn->peerAddress().toIpPort() << " -> "
    << conn->localAddress().toIpPort() << " is "
    << (conn->connected() ? "UP" : "DOWN");
}

void MessageServer::send(const TcpConnectionPtr& conn, std::string method, std::string message) {
  _codec.send(conn, method, message);
}

void MessageServer::logup(Ctx ctx) {
  printf("hello\n");
  send(ctx.conn, "/logup", "success");
}

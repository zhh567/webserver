#include <signal.h>
#include <webserver/logger/Logger.h>
#include <webserver/net/TcpServer.h>

using namespace webserver;

namespace {

void default_connection_callback(const TcpConnectionPtr& conn) {
  LOG_DEBUG << conn->get_peer_addr()->to_string() << " -> "
            << conn->get_local_addr()->to_string() << " is "
            << (conn->is_connected() ? "UP" : "DOWN");
}
void default_read_callback(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp time_point) {
  size_t len = buf->readable_bytes();
  char* read = buf->begin_read();
  std::string msg(read, len);
  LOG_DEBUG << "TcpConnection::read_callback [" << conn->get_conn_id()
            << "] bytes: " << buf->readable_bytes() << " : " << msg;

  buf->retrieve_all();
}

}  // namespace

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, listenAddr)),
      thread_pool_(std::make_unique<EventLoopThreadPool>(4, loop)),
      started_(false),
      next_conn_id_(1),
      connection_callback_(default_connection_callback),
      read_callback_(default_read_callback) {
  acceptor_->set_new_connection_callback(std::bind(&TcpServer::new_connection,
                                                   this, std::placeholders::_1,
                                                   std::placeholders::_2));
  LOG_DEBUG << "TcpServer::ctor[" << this << "] at " << this
            << " fd=" << acceptor_->get_fd();
}
TcpServer::~TcpServer() {
  // release all tcp connections
  for (auto& iter : connections_) {
    TcpConnectionPtr conn = iter.second;
    iter.second.reset();
    conn->get_loop()->run_in_loop(
        std::bind(&TcpConnection::connect_destroyed, conn));
    conn.reset();
  }

  LOG_DEBUG << "TcpServer::dtor[" << this << "] at " << this
            << " fd=" << acceptor_->get_fd();
}

// Acceptor accept a new connection and call this Callback
void TcpServer::new_connection(int sockfd, const InetAddress peerAddr) {
  loop_->abort_not_in_loop_thread();
  EventLoop* io_loop = thread_pool_->get_next_loop();

  // char buf[32] = {0};
  // std::snprintf(buf, sizeof buf, "%d", next_conn_id_);
  // ++next_conn_id_;

  LOG_INFO << "TcpServer::new_connection [" << this << "] - new connection ["
           << next_conn_id_ << "] from " << peerAddr.to_string();

  TcpConnectionPtr conn = std::make_shared<TcpConnection>(
      io_loop, sockfd, acceptor_->get_local_addr(), peerAddr, next_conn_id_);
  connections_[next_conn_id_] = conn;

  conn->set_close_callback(
      std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
  // define by
  conn->set_connection_callback(connection_callback_);
  conn->set_read_callback(read_callback_);
  conn->set_write_complete_callback(write_complete_callback_);
  // conn->set_hight_water_callback(hight_water_callback_);
  io_loop->run_in_loop(std::bind(&TcpConnection::connect_established, conn));
  ++next_conn_id_;
}

/// @brief This is channel's `close_callback_` in new TcpConection
/// @param conn
void TcpServer::remove_connection(const TcpConnectionPtr& conn) {
  loop_->run_in_loop(
      std::bind(&TcpServer::remove_connection_in_loop, this, conn));
}
void TcpServer::remove_connection_in_loop(const TcpConnectionPtr& conn) {
  loop_->abort_not_in_loop_thread();
  if (connections_.erase(conn->get_conn_id()) != 1) {
    LOG_WARN << "TcpServer::remove_connection_in_loop [" << this
             << "] - connection not found";
  }
  conn->get_loop()->run_in_loop(
      std::bind(&TcpConnection::connect_destroyed, conn));
}

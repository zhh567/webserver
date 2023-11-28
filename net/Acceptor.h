#ifndef WEBSERVER_NET_ACCEPTOR_H
#define WEBSERVER_NET_ACCEPTOR_H

#include <webserver/io/Channel.h>
#include <webserver/net/Socket.h>

#include <atomic>

namespace webserver {

class EventLoop;
class InetAddress;

class Acceptor {
 public:
  using NewConnectionCallback = std::function<void(int, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr);
  ~Acceptor();

  void listen();

  void set_new_connection_callback(const NewConnectionCallback& cb) {
    new_connection_callback_ = cb;
  }
  bool is_listening() const { return listening_; }
  int get_fd() const { return listen_fd_; }
  InetAddress get_local_addr() const { return local_addr_; }

 private:
  void handle_read();

  // std::atomic_int counter_;

  EventLoop* loop_;
  bool listening_;

  int idle_fd_;
  int listen_fd_;
  Socket listen_socket_;
  Channel listen_channel_;
  NewConnectionCallback new_connection_callback_;

  InetAddress local_addr_;
};
}  // namespace webserver

#endif  // WEBSERVER_NET_ACCEPTOR_H

#ifndef WEBSERVER_NET_SOCKET_H
#define WEBSERVER_NET_SOCKET_H

#include <webserver/net/InetAddress.h>

namespace webserver {

class Socket {
 public:
  Socket();
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  int fd() const { return sockfd_; }

  bool bind(const InetAddress& localAddress);
  bool listen();
  int accept(InetAddress* peerAddress);

  bool shutdown_write();
  bool set_tcp_no_delay(bool on);
  bool set_reuse_addr(bool on);
  bool set_reuse_port(bool on);
  bool set_keep_alive(bool on);

 private:
  const int sockfd_;
};

}  // namespace webserver

#endif  // WEBSERVER_NET_SOCKET_H

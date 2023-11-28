#ifndef WEBSERVER_NET_INETADDRESS_H
#define WEBSERVER_NET_INETADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <webserver/base/noncopyable.h>

#include <string>

namespace webserver {
  
class InetAddress {
 public:
  InetAddress(uint16_t port, bool loopbackOnly);
  InetAddress(const std::string& ip, uint16_t port);
  InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}

  const struct sockaddr_in& get_addr() const { return addr_; }

  std::string to_string() const;

 private:
  struct sockaddr_in addr_;
};

}  // namespace webserver

#endif  // WEBSERVER_NET_INETADDRESS_H
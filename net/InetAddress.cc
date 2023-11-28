#include <webserver/logger/Logger.h>
#include <webserver/net/InetAddress.h>

#include <cstring>

using namespace webserver;

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

InetAddress::InetAddress(uint16_t port, bool loopbackOnly) {
  std::memset(&addr_, 0, sizeof addr_);
  addr_.sin_family = AF_INET;
  in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
  addr_.sin_addr.s_addr = htonl(ip);
  addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  std::memset(&addr_, 0, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
    LOG_FATAL << "analyse ip fail: "
              << "ip: [" << ip << "] port: [" << port << "]";
    addr_.sin_addr.s_addr = kInaddrLoopback;
  }
}

std::string InetAddress::to_string() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
  uint16_t port = ntohs(addr_.sin_port);
  snprintf(buf + std::strlen(buf), sizeof(buf) - std::strlen(buf), ":");
  snprintf(buf + std::strlen(buf), sizeof(buf) - std::strlen(buf), "%d", port);
  return buf;
}

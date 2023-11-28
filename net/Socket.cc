#include <netinet/tcp.h>
#include <unistd.h>
#include <webserver/net/Socket.h>

using namespace webserver;

Socket::Socket() : sockfd_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {}
Socket::~Socket() { ::close(sockfd_); }

bool Socket::bind(const InetAddress& localAddress) {
  return ::bind(sockfd_, (struct sockaddr*)(&localAddress.get_addr()),
                sizeof(sockaddr_in)) == 0;

  // int ret = ::bind(sockfd_, (struct sockaddr*)&localaddr.getSockAddrInet(),
  //  sizeof(struct sockaddr_in));
}

bool Socket::listen() { return ::listen(sockfd_, SOMAXCONN) == 0; }

int Socket::accept(InetAddress* peerAddress) {
  if (peerAddress != nullptr) {
    socklen_t len = sizeof(sockaddr_in);
    return ::accept(sockfd_,
                    reinterpret_cast<sockaddr*>(
                        const_cast<sockaddr_in*>(&peerAddress->get_addr())),
                    &len);
  } else {
    return ::accept(sockfd_, nullptr, nullptr);
  }
}

bool Socket::shutdown_write() { return ::shutdown(sockfd_, SHUT_WR) == 0; }

bool Socket::set_tcp_no_delay(bool on) {
  int optval = on ? 1 : 0;
  return ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
                      static_cast<socklen_t>(sizeof optval)) == 0;
}

bool Socket::set_reuse_addr(bool on) {
  int optval = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
                      static_cast<socklen_t>(sizeof optval)) == 0;
}

bool Socket::set_reuse_port(bool on) {
#ifdef SO_REUSEPORT
  int optval = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                      static_cast<socklen_t>(sizeof optval)) == 0;
#else
  return false;
#endif
}

bool Socket::set_keep_alive(bool on) {
  int optval = on ? 1 : 0;
  return ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
                      static_cast<socklen_t>(sizeof optval)) == 0;
}

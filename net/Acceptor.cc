#include <fcntl.h>
#include <unistd.h>
#include <webserver/io/EventLoop.h>
#include <webserver/logger/Logger.h>
#include <webserver/net/Acceptor.h>

#include <cstring>

using namespace webserver;

namespace {

int create_idle_fd() {
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) LOG_FATAL << "create socket error";
  return fd;
}

static std::atomic_int total_counter = 0;
static std::atomic_int good_counter = 0;
static std::atomic_int bad_counter = 0;
static std::atomic_int toomany_counter = 0;

}  // namespace

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr)
    : loop_(loop),
      listening_(false),
      idle_fd_(::open("/dev/null", O_CLOEXEC)),
      listen_fd_(::create_idle_fd()),
      listen_socket_(listen_fd_),
      listen_channel_(loop, listen_fd_),
      local_addr_(listen_addr) {
  listen_socket_.set_reuse_port(false);
  listen_socket_.bind(listen_addr);
  listen_socket_.set_reuse_addr(true);

  listen_channel_.set_read_callback(std::bind(&Acceptor::handle_read, this));
  listen_channel_.enable_reading();

  loop_->set_is_net_loop(true);

  // counter_ = 0;

  // LOG_DEBUG << "Acceptor::ctor[" << this << "] at " << this
  //           << " fd=" << listen_fd_
  //           << " channel status: " << listen_channel_.get_status();
}
Acceptor::~Acceptor() {
  listen_channel_.disable_all();
  listen_channel_.remove();
  ::close(listen_fd_);
  ::close(idle_fd_);

  LOG_DEBUG << "Acceptor::dtor[" << this << "] at " << this
            << " fd=" << listen_fd_ << " total accept: " << total_counter
            << " good: " << good_counter << " bad: " << bad_counter
            << " too many: " << toomany_counter;
}

void Acceptor::listen() {
  listen_socket_.listen();
  listening_ = true;
  listen_channel_.enable_reading();
}

/// @brief This is `listen_channel_`'s ReadEventCallback
void Acceptor::handle_read() {
  InetAddress peer_address(1, true);
  int fd = listen_socket_.accept(&peer_address);
  total_counter++;
  if (fd > 0) {
    good_counter++;
    LOG_INFO << "accept a new connection from " << peer_address.to_string();
    if (new_connection_callback_) {
      new_connection_callback_(fd, peer_address);
    }
  }
  // handle error
  else if (fd < 0 && errno == EMFILE) {
    toomany_counter++;
    LOG_ERROR << "accept too many connection";
    ::close(idle_fd_);
    idle_fd_ = ::accept(listen_fd_, nullptr, nullptr);
    ::close(idle_fd_);
    idle_fd_ = ::open("/dev/null", O_CLOEXEC | O_RDONLY);
  } else {
    bad_counter++;
    LOG_ERROR << "accept error: " << std::strerror(errno);
  }
}

#include <webserver/net/TcpConnection.h>

#include <cstring>

using namespace webserver;

TcpConnection::TcpConnection(EventLoop* loop, int socket_fd,
                             InetAddress local_addr, InetAddress peer_addr,
                             int conn_id)
    : loop_(loop),
      state_(State::kDisconnecting),
      socket_(std::make_unique<Socket>(socket_fd)),
      channel_(std::make_unique<Channel>(loop, socket_fd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      conn_id_(conn_id),
      hight_water_(64 * 1024 * 1024) {
  LOG_DEBUG << "TcpConnection::ctor[" << conn_id_ << "] at " << this
            << " fd=" << socket_fd;
  socket_->set_keep_alive(true);
  channel_->set_read_callback(
      std::bind(&TcpConnection::handle_read, this, std::placeholders::_1));
  channel_->set_write_callback(std::bind(&TcpConnection::handle_write, this));
  channel_->set_close_callback(std::bind(&TcpConnection::handle_close, this));
  channel_->set_error_callback(std::bind(&TcpConnection::handle_error, this));
  std::string s("TcpConnection");
  s.append(std::to_string(conn_id_));
  channel_->set_name(s);
}
TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << conn_id_ << "] at " << this
            << " fd=" << channel_->fd();
}

/// This method make `send_in_loop` run in loop thread. If not in loop thread,
/// appent it to `pending_functions`
void TcpConnection::send(char* data, size_t len) {
  if (state_ == State::kConnected) {
    if (loop_->is_in_loop_thread()) {
      send_in_loop(data, len);
    } else {
      loop_->run_in_loop(
          std::bind(&TcpConnection::send_in_loop, this, data, len));
    }
  } else {
    LOG_ERROR << "TcpConnection::send() not connected, give up writing";
  }
}
/// `EventLoop` run it as `pending_function`. Append data to `write_buffer_` and
/// `enable_write()`.
void TcpConnection::send_in_loop(char* data, size_t len) {
  loop_->abort_not_in_loop_thread();
  if (state_ == State::kDisconnected) {
    LOG_WARN << "disconnected, give up writing";
    return;
  }

  /* In there, can send data directly if write buffer is empty. */

  size_t old_len = write_buffer_.readable_bytes();
  // When buffer is insufficient, call `hight_water_callback_`
  if (old_len + len >= hight_water_ && old_len < hight_water_ &&
      hight_water_callback_) {
    loop_->run_in_loop(
        std::bind(hight_water_callback_, this->get_shared_ptr()));
  }
  // write data to buffer, and make Poller listen write event.
  write_buffer_.append(data, len);
  if (!channel_->is_writing()) {
    channel_->enable_writing();
  }
}

void TcpConnection::shutdown() {
  if (state_ == State::kConnected) {
    set_state(State::kDisconnecting);
    loop_->run_in_loop(
        std::bind(&TcpConnection::shutdown_in_loop, this->get_shared_ptr()));
  }
}
void TcpConnection::shutdown_in_loop() {
  loop_->abort_not_in_loop_thread();
  if (!channel_->is_writing()) {
    socket_->shutdown_write();
  }
}

void TcpConnection::force_close() {
  if (state_ == State::kConnected || state_ == State::kDisconnecting) {
    set_state(State::kDisconnecting);
    loop_->run_in_loop(
        std::bind(&TcpConnection::force_close_in_loop, this->get_shared_ptr()));
  }
}
void TcpConnection::force_close_in_loop() {
  loop_->abort_not_in_loop_thread();
  if (state_ == State::kConnected || state_ == State::kDisconnecting) {
    handle_close();
  }
}

void TcpConnection::connect_established() {
  loop_->abort_not_in_loop_thread();
  connection_callback_(this->shared_from_this());
  set_state(State::kConnected);
  channel_->enable_reading();
}
void TcpConnection::connect_destroyed() {
  loop_->abort_not_in_loop_thread();
  if (state_ == State::kConnected) {
    set_state(State::kDisconnecting);
    // channel_->disable_all();
    // `disable_all` method disable all events and remove from epoll, it's
    // repeat with `channel_->remove()`
  }
  LOG_DEBUG << "connection destroy and remove channel";
  channel_->remove();
}

// These are channel's callback.

void TcpConnection::handle_read(Timestamp recv_time) {
  loop_->abort_not_in_loop_thread();

  int saved_errno = 0;
  size_t n = read_buffer_.read_fd(channel_->fd(), &saved_errno);

  if (n > 0) {
    // `read_callback_` is set by TcpServer.
    // TcpServer's callback can set by user.
    read_callback_(this->get_shared_ptr(), &read_buffer_, recv_time);
  } else if (n == 0) {
    LOG_DEBUG << "TcpConnection::handle_read read 0 bytes, "
              << channel_->get_name();
    handle_close();
  } else {
    LOG_ERROR << "TcpConnection::handle_read fail: " << strerror(saved_errno);
    handle_error();
  }
}

void TcpConnection::handle_write() {
  loop_->abort_not_in_loop_thread();
  if (channel_->is_writing()) {
    size_t n = ::write(socket_->fd(), write_buffer_.begin_read(),
                       write_buffer_.readable_bytes());
    // write successfully
    if (n >= 0) {
      write_buffer_.retrieve(n);
      if (write_buffer_.readable_bytes() == 0) {
        channel_->disable_writing();
        if (write_complete_callback_) {
          loop_->run_in_loop(
              std::bind(write_complete_callback_, this->get_shared_ptr()));
        }
        if (state_ == State::kDisconnecting) {
          shutdown_in_loop();
        }
      }
    }
    // write fail
    else {
      LOG_ERROR << "TcpConnection::handle_write fail: " << strerror(errno);
    }

  } else {
    LOG_ERROR << "TcpConnection::handle_write() fd = " << channel_->fd()
              << " is down, no more writing";
  }
}

void TcpConnection::handle_close() {
  loop_->abort_not_in_loop_thread();
  if (state_ == State::kConnected || state_ == State::kDisconnecting) {
    set_state(State::kDisconnected);
  } else {
    LOG_ERROR << "TcpConnection::handle_close() fd = " << channel_->fd();
  }

  channel_->disable_all();
  close_callback_(this->get_shared_ptr());
}

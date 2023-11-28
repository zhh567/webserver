#ifndef WEBSERVER_NET_TCPCONNECTION_H
#define WEBSERVER_NET_TCPCONNECTION_H

#include <webserver/base/Alias.h>
#include <webserver/base/Buffer.h>
#include <webserver/io/Channel.h>
#include <webserver/io/EventLoop.h>
#include <webserver/logger/Logger.h>
#include <webserver/net/InetAddress.h>
#include <webserver/net/Socket.h>

#include <memory>

namespace webserver {

// class EventLoop;
// class Channel;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  enum class State { kConnecting, kConnected, kDisconnecting, kDisconnected };

  TcpConnection(EventLoop* loop, int socket_fd, InetAddress local_addr,
                InetAddress peer_addr, int conn_id);
  ~TcpConnection();

  std::shared_ptr<TcpConnection> get_shared_ptr() {
    return shared_from_this();
  }

  EventLoop* get_loop() { return loop_; }
  InetAddress* get_local_addr() { return &local_addr_; }
  InetAddress* get_peer_addr() { return &peer_addr_; }
  bool is_connected() const { return state_ == State::kConnected; }
  void set_tcp_nodelay(bool on) { socket_->set_tcp_no_delay(on); }

  void send(char* data, size_t len);
  void shutdown();
  void force_close();

  // set callback
  void set_connection_callback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
  }
  void set_read_callback(const ReadCallback& cb) { read_callback_ = cb; }
  void set_write_complete_callback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = cb;
  }
  void set_close_callback(const CloseCallback& cb) { close_callback_ = cb; }
  void set_hight_water_callback(const HightWaterCallback& cb) {
    hight_water_callback_ = cb;
  }

  void set_hight_water(int hight_water) { hight_water_ = hight_water; }
  Buffer* get_read_buffer() { return &read_buffer_; }
  Buffer* get_write_buffer() { return &write_buffer_; }
  int get_conn_id() { return conn_id_; }

  // only call once
  void connect_established();
  void connect_destroyed();

 private:
  void handle_read(Timestamp);
  void handle_write();
  void handle_close();
  void handle_error() {
    LOG_ERROR << "TcpConnection::handle_error";
    handle_close();
  }
  void send_in_loop(char* data, size_t len);
  void shutdown_in_loop();
  void force_close_in_loop();

  void set_state(State state) { state_ = state; }

  int conn_id_;

  EventLoop* loop_;
  State state_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress local_addr_;
  InetAddress peer_addr_;

  ConnectionCallback connection_callback_;
  ReadCallback read_callback_;
  WriteCompleteCallback write_complete_callback_;
  CloseCallback close_callback_;
  HightWaterCallback hight_water_callback_;

  int hight_water_;

  Buffer read_buffer_;
  Buffer write_buffer_;
};
}  // namespace webserver

#endif  // WEBSERVER_NET_TCPCONNECTION_H

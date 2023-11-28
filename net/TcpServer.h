#ifndef WEBSERVER_NET_TCPSERVER_H
#define WEBSERVER_NET_TCPSERVER_H

#include <webserver/net/Acceptor.h>
#include <webserver/net/TcpConnection.h>
#include <webserver/thread/EventLoopThreadPool.h>

#include <map>
#include <string>

namespace webserver {

class TcpServer {
 public:
  using TcpConnectionMap = std::map<int, TcpConnectionPtr>;

  TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  ~TcpServer();

  void start() {
    bool expect = false;
    if (started_.compare_exchange_strong(expect, true)) {  // run only once
      thread_pool_->start(thread_init_callback_);
      acceptor_->listen();
    }
  }

  EventLoop* get_loop() { return loop_; }
  EventLoopThreadPool* get_thread_pool() { return thread_pool_.get(); }

  void set_connection_callback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
  }
  void set_read_callback(const ReadCallback& cb) { read_callback_ = cb; }
  void set_write_complete_callback(const WriteCompleteCallback& cb) {
    write_complete_callback_ = cb;
  }
  // void set_close_callback(const CloseCallback& cb) { close_callback_ = cb; }
  // void set_hight_water_callback(const HightWaterCallback& cb) {
  //   hight_water_callback_ = cb;
  // }
  void set_thread_init_callback(const EventLoopThreadInitCallback& cb) {
    thread_init_callback_ = cb;
  }
  void set_thread_num(int num) { thread_pool_->set_thread_num(num); }

 private:
  // acceptor_'s new_connection_callback
  void new_connection(int sockfd, const InetAddress peerAddr);
  // TcpConnection's close_callback
  void remove_connection(const TcpConnectionPtr& conn);
  void remove_connection_in_loop(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  std::unique_ptr<Acceptor> acceptor_;
  std::unique_ptr<EventLoopThreadPool> thread_pool_;

  // `TcpConnection` call them in handle* methods which called by `Channel`
  // TcpServer.*Callback <- TcpConnection.handle* <- Channel.*Callback

  ConnectionCallback connection_callback_;
  ReadCallback read_callback_;
  WriteCompleteCallback write_complete_callback_;
  // CloseCallback close_callback_;
  // HightWaterCallback hight_water_callback_;

  EventLoopThreadInitCallback thread_init_callback_;

  // always in loop thread
  int next_conn_id_;  // increment only
  TcpConnectionMap connections_;
  std::atomic_bool started_;
};

}  // namespace webserver

#endif  // WEBSERVER_NET_TCPSERVER_H

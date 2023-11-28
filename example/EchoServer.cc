#include <signal.h>
#include <webserver/io/EventLoop.h>
#include <webserver/logger/Logger.h>
#include <webserver/net/TcpServer.h>

#include <iostream>

webserver::EventLoop* kLoop;

void thread_init_func(webserver::EventLoop* loop) {
  ::write(0, "thread init\n", 13);
}

void sigterm_handler(int signal) {
  ::write(0, "recv SIGTERM\n", 14);
  kLoop->quit();
}

void echo_callback(const TcpConnectionPtr& conn, Buffer* buf,
                   Timestamp time_point) {
  size_t len = buf->readable_bytes();
  char* read = buf->begin_read();
  std::string msg(read, len);
  LOG_DEBUG << "TcpConnection::read_callback [" << conn->get_conn_id()
            << "] bytes: " << buf->readable_bytes() << " : " << msg;

  conn->send(read, len);

  buf->retrieve_all();
}

int main(int argc, char const* argv[]) {
  webserver::InetAddress addr("0.0.0.0", 8080);
  webserver::EventLoop loop;
  kLoop = &loop;
  webserver::TcpServer server(&loop, addr);
  server.set_thread_init_callback(thread_init_func);
  server.set_thread_num(2);
  server.set_read_callback(echo_callback);
  server.start();

  struct sigaction sa;
  sa.sa_handler = sigterm_handler;
  sa.sa_flags = 0;
  if (sigaction(SIGTERM, &sa, nullptr) == -1) {
    perror("sigaction");
    return 1;
  }

  loop.loop();

  return 0;
}

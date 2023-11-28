#ifndef WEBSERVER_BASE_BUFFER_H
#define WEBSERVER_BASE_BUFFER_H

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace webserver {

class Buffer {
 public:
  Buffer()
      : buffer_(kCheapPrepend + kInitialSize),
        read_index_(kCheapPrepend),
        write_index_(kCheapPrepend) {}
  Buffer(Buffer&& buffer)
      : buffer_(std::move(buffer.buffer_)),
        read_index_(buffer.read_index_),
        write_index_(buffer.write_index_) {}

  size_t readable_bytes() const { return write_index_ - read_index_; }
  size_t writable_bytes() const { return buffer_.size() - write_index_; }
  size_t prependable_bytes() const { return read_index_; }

  void swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(read_index_, rhs.read_index_);
    std::swap(write_index_, rhs.write_index_);
  }

  const char* find(const char* data, size_t len) {
    auto result = std::search(begin_read(), begin_write(), data, data + len);
    return result == begin_write() ? nullptr : result;
  }

  // release readed space
  void retrieve(size_t len) {
    if (len < readable_bytes()) {
      read_index_ += len;
    } else {
      retrieve_all();
    }
  }

  void retrieve_all() {
    read_index_ = kCheapPrepend;
    write_index_ = kCheapPrepend;
  }

  std::string retrieve_as_string(size_t len) {
    std::string result(static_cast<const char*>(begin_read()), len);
    retrieve(len);
    return result;
  }

  // write
  void append(const char* data, size_t len) {
    if (len > writable_bytes()) {
      make_space(len);
    }
    std::copy(data, data + len, begin_write());
    write_index_ += len;
  }

  void append(const std::string data) { append(data.c_str(), data.size()); }

  // unwrite
  void unwrite(size_t len) {
    if (len < readable_bytes()) {
      write_index_ -= len;
    } else {
      retrieve_all();
    }
  }

  // read
  size_t peek(void* data, size_t len) {
    if (len > readable_bytes()) {
      len = readable_bytes();
    }
    char* d = static_cast<char*>(data);
    std::copy(begin_read(), begin_read() + len, d);
    return len;
  }

  // read from file_descriptor
  ssize_t read_fd(int fd, int* saved_errno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writable_bytes();
    vec[0].iov_base = begin_write();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    ssize_t n{0};
    do {
      n = readv(fd, vec, 2);
    } while (n == -1 &&
             errno == EINTR);  // when interrupted by signal, continue read

    if (n < 0) {
      *saved_errno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
      write_index_ += n;
    } else {
      write_index_ = buffer_.size();
      append(extrabuf, n - writable);
    }
    return n;
  }

  char* begin() { return &*buffer_.begin(); }
  char* begin_read() { return begin() + read_index_; }
  char* begin_write() { return begin() + write_index_; }

 private:
  void make_space(size_t len) {
    if (read_index_ > kCheapPrepend) {
      size_t readable = readable_bytes();
      std::copy(begin() + read_index_, begin() + write_index_,
                begin() + kCheapPrepend);
      read_index_ = kCheapPrepend;
      write_index_ = kCheapPrepend + readable;
    }
    if (writable_bytes() < len) {
      // when buffer is less than len, resize space needed
      buffer_.resize(write_index_ + len);
    }
  }

 private:
  std::vector<char> buffer_;
  size_t read_index_;
  size_t write_index_;

  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;
};

}  // namespace webserver
#endif  // WEBSERVER_BASE_BUFFER_H

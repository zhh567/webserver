import threading
import socket
import time


def thread_func():
    for i in range(10):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
        s.connect(('192.168.56.121', 8080))

        s1 = 'a'*100+'='
        s.send(s1.encode(encoding='utf-8'))
        print(threading.current_thread().name, ' -> ', s.recv(len(s1)))
        time.sleep(0.01)

        s2 = 'b'*100+'='
        s.send(s2.encode(encoding='utf-8'))
        print(threading.current_thread().name, ' -> ', s.recv(len(s2)))

        s3 = 'c'*100+'='
        s.send(s2.encode(encoding='utf-8'))
        print(threading.current_thread().name, ' -> ', s.recv(len(s2)))


if __name__ == '__main__':
    tt = []
    for i in range(10):
        t = threading.Thread(target=thread_func, name='thread-{}'.format(i))
        t.start()
        tt.append(t)
    map(lambda t: t.join(), tt)

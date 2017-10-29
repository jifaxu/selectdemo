#include <iostream>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sstream>
#include "csapp.h"


#define NUM_THREADS 2

#define PRODUCE_MSG_INTERVAL 10

#define MSG_COUNT  5

void *connectAndSend(void *);
void *waitForFd(void *);

void singleConnection() ;

// 对应前置为多线程或者多路复用的情况
void multiConnect() ;

using namespace std;



pthread_barrier_t pthread_barrier;

int main() {


//    pthread_exit(NULL);


    time_t start, end;

    start = time(NULL);


//    singleConnection();
    multiConnect();

    end = time(NULL);

    cout << (long) (end - start)<< endl;

}

void singleConnection() {


    for (int num = NUM_THREADS; num >= 1; num--) {
        int fd = open_clientfd("localhost", 9999);

        pthread_t pid;
        pthread_barrier_init(&pthread_barrier, NULL, 2);
        pthread_create(&pid, NULL, waitForFd, &fd);
        if (fd == -1) {
            cout << "error " << endl;
        }

        for (int i = MSG_COUNT; i >= 0; i--) {

            stringstream ss;
            cout << to_string(num * 10 + i) << " from " << num << endl;
            string msg = to_string(num * 10 + i) + "\n";
            rio_writen(fd, (void *) msg.c_str(), msg.length());
            sleep(PRODUCE_MSG_INTERVAL);
        }

        pthread_barrier_wait(&pthread_barrier);
    }

}


// 对应前置为多线程或者多路复用的情况
void multiConnect() {
     pthread_t threads[NUM_THREADS];
    int rc;
    int i;


    time_t start, end;

    start = time(NULL);


    pthread_barrier_init(&pthread_barrier, NULL, 2 * NUM_THREADS + 1);


    int v[NUM_THREADS];
    for (i = 1; i <= NUM_THREADS; i++) {
        v[i] = i;
        rc = pthread_create(&threads[i], NULL, connectAndSend, &v[i]);

        if (rc) {
            exit(-1);
        }
    }

    pthread_barrier_wait(&pthread_barrier);

    end = time(NULL);

    cout << (long) (end - start)<< endl;

}

// 获得连接, 然后假设每隔 1s 发送一条给主机
void *connectAndSend(void *pVoid) {

    int num = *(int *)pVoid;

    int fd = open_clientfd("localhost", 9999);

    pthread_t pid;
    pthread_create(&pid, NULL, waitForFd, &fd);
    if (fd == -1) {
        cout << "error " << endl;
    }

    for (int i = MSG_COUNT; i >= 0; i--) {

        stringstream ss;
        cout << to_string(num * 10 + i) << " from " << num << endl;
        string msg = to_string(num * 10 + i) + "\n";
        send(fd, msg.c_str(), msg.length(), 0);
        rio_writen(fd, (void *) msg.c_str(), msg.length());
        sleep(PRODUCE_MSG_INTERVAL);
    }

    cout << num << " all sent" << endl;

    pthread_barrier_wait(&pthread_barrier);
}

void *waitForFd(void *arg) {

    int fd = *(int *)arg;

    int n;
    rio_t rio;
    Rio_readinitb(&rio, fd);
    char buf[MAXLINE];
    int count = 0;
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {

        cout << "recv " << buf << endl;
        count++;
        if (count == MSG_COUNT + 1) {

            close(fd);
            break;
        }
    }
    cout << fd  << "all recv" << endl;

    pthread_barrier_wait(&pthread_barrier);

}


#include <iostream>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sstream>
#include "csapp.h"


#define NUM_THREADS 2

#define PRODUCE_MSG_INTERVAL 2


void *connectAndSend(void *);

using namespace std;



pthread_barrier_t pthread_barrier;

int main() {

    pthread_t threads[NUM_THREADS];
    int rc;
    int i;


    time_t start, end;

    start = time(NULL);
    cout << clock();


    pthread_barrier_init(&pthread_barrier, NULL, NUM_THREADS + 1);


    int v[NUM_THREADS];
    for (i = 0; i < NUM_THREADS; i++) {
        v[i] = i;
        rc = pthread_create(&threads[i], NULL, connectAndSend, &v[i]);

        if (rc) {
            exit(-1);
        }
    }

    pthread_barrier_wait(&pthread_barrier);

    end = time(NULL);



    cout << (end - start) / 1000<< endl;

//    pthread_exit(NULL);
}

// 获得连接, 然后假设每隔 1s 发送一条给主机
void *connectAndSend(void *pVoid) {

    int num = *(int *)pVoid;


//    cout << t->num << endl;

    int fd = open_clientfd("localhost", 9999);


    if (fd == -1) {
        cout << "error " << endl;
    }

    rio_t rio;
    char buf[MAXLINE];

    Rio_readinitb(&rio, fd);


    for (int i = 0; i < 5; i++) {

        stringstream ss;
        ss << i << " st from " << num << "\n";

        string msg = ss.str();

        send(fd, msg.c_str(), msg.length(), 0);

        rio_readlineb(&rio, buf, MAXLINE);

        sleep(PRODUCE_MSG_INTERVAL);
    }

    close(fd);

    pthread_barrier_wait(&pthread_barrier);
}


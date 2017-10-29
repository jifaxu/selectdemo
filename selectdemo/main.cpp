#include <iostream>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "csapp.h"
#include <cstring>

using namespace std;


#define WORKTIME 1
typedef struct {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    rio_t clientrio[FD_SETSIZE];
} pool;
typedef struct sockaddr SA;

int open_listenfd(int port);

void add_client(int connfd, pool *p);

void readFromReadyClients(pool *p);

void init_pool(int listenfd, pool *p);

//
int multiplexing() ;

void addIntoMq(char* msg, int connfd);

int single() ;

void readFromClient(int connfd) ;

#define LISTENQ  1024  /* second argument to listen() */



int main() {

    single();
//    multiplexing();
}

// 阻塞 --------------------------------------
int single() {
    int listenfd, connfd;
    socklen_t  clientlen;
    struct sockaddr_storage clientaddr;

    char client_hostname[MAXLINE], client_port[MAXLINE];

    listenfd = open_listenfd(9999);

    while (true) {

        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);
        getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        readFromClient(connfd);
    }
}

void readFromClient(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);

    string msg = "ok";
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
//        Rio_writen(connfd, buf, n);

        printf("%s\n", buf);
        addIntoMq(buf, connfd);
    }
}

// 阻塞 --------------------------------------


// 多路复用 --------------------------------------
int multiplexing() {

    int listenfd, connfd;
    socklen_t  clientlen;
    struct sockaddr_storage clientaddr;

    static pool pool;


    listenfd = open_listenfd(9999);

    init_pool(listenfd, &pool);

    while(1) {
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = accept(listenfd, (SA *) &clientaddr, &clientlen);

            add_client(connfd, &pool);
        }

        readFromReadyClients(&pool);
    }
}

void init_pool(int listenfd, pool *p) {

    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        p->clientfd[i] = -1;
    }

    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p) {
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++) {
        if (p->clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            rio_readinitb(&p->clientrio[i], connfd);

            FD_SET(connfd, &p->read_set);

            if (connfd > p->maxfd) {
                p->maxfd = connfd;
            }
            if (i > p->maxi) {
                p->maxi = i;
            }
            break;
        }
    }
    if (i == FD_SETSIZE) {
        app_error("add_client : too many clients");
    }
}

void readFromReadyClients(pool *p) {

    int i, connfd, n;

    char buf[MAXLINE];
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {


        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
            p->nready--;
            if ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
                cout << buf << endl;
                addIntoMq(buf, connfd);
            } else {
                close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }

}

// 多路复用 --------------------------------------

// 插入 mq, 处理结束后再通过 connfd 返回消息给 client
void addIntoMq(char* msg, int connfd) {

    sleep(WORKTIME);

    string ret_msg = "ok\n";

    rio_writen(connfd, (void *) ret_msg.c_str(), 3);
}

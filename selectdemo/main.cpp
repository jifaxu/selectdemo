#include <iostream>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "csapp.h"
#include <map>
#include <queue>

using namespace std;

void *readFromRecvFd(void *arg);

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

void connectToBack() ;

void initReadThread(int ) ;

// 可以乱序
void readFromReadyClients2(pool *p) ;

#define LISTENQ  1024  /* second argument to listen() */



int main() {


//    single();
    multiplexing();
}

int send_fd;
int recv_fd;


int single_fd;

int is_single;

map<char, int> to_fd;

void connectToBack(int single) {

    send_fd = open_clientfd("localhost", 10000);
    recv_fd = open_clientfd("localhost", 10001);

    initReadThread(single);

    if (send_fd == -1 || recv_fd == -1) {
        cout << "error" << endl;
        exit(-1);
    }


}

void initReadThread(int single) {

    is_single = single;
    pthread_t pid;

    pthread_create(&pid, NULL, readFromRecvFd, &single);


}

void *readFromRecvFd(void *arg) {

    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, recv_fd);

    string msg = "ok";

    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {

//        printf("%s\n", buf);
        int i = atoi(buf);

        char k = buf[0];

        if (is_single == 1) {

            send(single_fd, buf, n, 0);

        } else {

            cout << "ret" << buf << endl;
//            cout << "oo fd: " << to_fd[k] << endl;
            cout << buf << endl;
            string msg = atoi(buf) + "\n";

            send(to_fd[k], buf, n, 0);
        }
    }

}


// 阻塞 --------------------------------------
int single() {

    connectToBack(1);
    int listenfd;
    socklen_t  clientlen;
    struct sockaddr_storage clientaddr;

    char client_hostname[MAXLINE], client_port[MAXLINE];

    listenfd = open_listenfd(9999);




    while (true) {

        clientlen = sizeof(struct sockaddr_storage);
        single_fd = accept(listenfd, (SA *) &clientaddr, &clientlen);
        getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        readFromClient(single_fd);
    }
}

void readFromClient(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);

    string msg = "ok";
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {

//        printf("single recv from client %s\n", buf);
        send(send_fd, buf, n, 0);
    }
}

// 阻塞 --------------------------------------


// 多路复用 --------------------------------------

//queue<int> m;
map<char, vector<int>> mm ;


int multiplexing() {

    connectToBack(0);

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

//        readFromReadyClients(&pool);
        readFromReadyClients2(&pool);
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

// 不可乱序
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
//                addIntoMq(buf, connfd);
                int i = atoi(buf);
                char k = buf[0];
                char c = buf[1];

                cout << "from client" << i << endl;

                if (mm.find(k) == mm.end()) {

                    vector<int> v;

                    v.push_back(i);
                    mm[k] = v;
                } else {
                    vector<int> v = mm[k];
                    v.push_back(i);
                    mm[k] = v;
                }
                if (c == '0') { // send all
                    vector<int> v = mm[k];
                    vector<int>::const_iterator iter = v.begin();
                    to_fd[k] = connfd;
                    cout << "fd: " << connfd << endl;
                    while (iter != v.end()) {

                        int v = *iter.base();

                        string msg = to_string(v) + "\n";

                        cout << " send to back : " << msg << endl;
                        send(send_fd, msg.c_str(), msg.length(), 0);
                        iter++;
                    }
                }
            } else {
                close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}

// 可以乱序
void readFromReadyClients2(pool *p) {

    int i, connfd, n;

    char buf[MAXLINE];
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {


        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
            p->nready--;
            if ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {



                to_fd[buf[0]] = connfd;
                send(send_fd, buf, n, 0);
            } else {
                close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}

// 多路复用 --------------------------------------

// 插入 mq, 处理结束后再通过 connfd 返回消息给 client, 假设这些一共需要耗时 WORKTIME
void addIntoMq(char* msg, int connfd) {

    sleep(WORKTIME);

    string ret_msg = "ok\n";

    rio_writen(connfd, (void *) ret_msg.c_str(), 3);
}

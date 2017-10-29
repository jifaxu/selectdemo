#include <iostream>
#include <vector>
#include <queue>

#include "csapp.h"

#define WORKTIME 1

void addIntoMq(char* msg) ;

using namespace std;

//vector<string> list;

queue<int> tasks;


void *readFromQueue(void * arg) ;

void initReadThread() ;

int listenfd, recv_fd, listenfd2, send_fd;

int main() {
    std::cout << "Hello, World!" << std::endl;


    initReadThread();


    socklen_t  clientlen;
    socklen_t  clientlen2;
    struct sockaddr_storage clientaddr;
    struct sockaddr_storage clientaddr2;

    char client_hostname[MAXLINE], client_port[MAXLINE];

    listenfd = open_listenfd(10000);
    listenfd2 = open_listenfd(10001);



    while (true) {

        clientlen = sizeof(struct sockaddr_storage);
        send_fd = accept(listenfd2, (SA *) &clientaddr2, &clientlen2);
        recv_fd = accept(listenfd, (SA *) &clientaddr, &clientlen);
        getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);


        size_t n;
        char buf[MAXLINE];
        rio_t rio;

        Rio_readinitb(&rio, recv_fd);

        string msg = "ok";
        while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
    //        Rio_writen(recv_fd, buf, n);

            printf("%s\n", buf);
            addIntoMq(buf);

            string str = "ok\n";
        }
    }

    return 0;
}

void initReadThread() {

    pthread_t pid;

    pthread_create(&pid, NULL, readFromQueue, nullptr);


}

void *readFromQueue(void * arg) {

    while (true) {
        while (tasks.empty()) ;

        int msg = tasks.front();
        tasks.pop();

        sleep(WORKTIME);
        string s = to_string(msg) + "\n";

        cout << "end" << s<< endl;

        send(send_fd, s.c_str(), s.length(), 0);
    }
}

void addIntoMq(char* msg) {

    printf("add into mq %s", msg);


    tasks.push(atoi(msg));
}

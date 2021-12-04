#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "server_lib.h"
#include "server_client.h"
#include "server_sqlite3.h"

uint16_t port;
int nthreads = 0;
struct cthread_arg* clients[MAXCLIENTS] = { NULL };

// kod serwera nasłuchowego TCP
void *tcpserver(void *arg) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
        LOG("Bind to port %hd/tcp failed", port);
        fprintf(stderr, "\nCannot start, check log\n");
        exit(2);
    }
    listen(sock, 5);
    for(;;) {
        struct sockaddr_in clientaddr;
        unsigned int sockaddr_size = sizeof(clientaddr);
        struct cthread_arg *arg = (struct cthread_arg *) calloc(1, sizeof(struct cthread_arg));
        arg->sock = accept(sock, (struct sockaddr *) &clientaddr, &sockaddr_size);
        if(arg->sock >= 0) {
            arg->sin_addr = clientaddr.sin_addr;
            pthread_t ctid;
            pthread_create(&ctid, NULL, cthread, arg);
        } else {
            LOG("Error on accepting new connection", port);
            free(arg);
        }
    }
}

// kod serwera nasłuchowego UDP
void *udpserver(void *arg) {
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
        LOG("Bind to port %hd/udp failed", port);
        fprintf(stderr, "\nCannot start, check log\n");
        exit(2);
    }
    for(;;) {
        struct sockaddr_in clientaddr;
        unsigned int sockaddr_size = sizeof(clientaddr);
        char buf[65536];
        int n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *) &clientaddr, &sockaddr_size);
        if(n > 0) {
            buf[n] = 0;
            uint8_t *ip = (uint8_t *) &clientaddr.sin_addr;
            LOG("UDP packet (%d bytes) from %d.%d.%d.%d\t%s", n, ip[0], ip[1], ip[2], ip[3], buf);
            // sendto(sock, buf, n, MSG_DONTWAIT, (struct sockaddr *) &clientaddr, sockaddr_size);
        }
    }
}

void printUser(int id, const char *login) {
    printf("%d\t%s\n", id, login);
}

int main(int argc, char* argv[]) {
    _log = fopen(LOGFNAME, "a");
    if(!_log) {
        _log = stderr;
        LOG("Unable to open log file %s", LOGFNAME);
    }
    if(argc < 2 || (port = atoi(argv[1])) <= 0) {
        fprintf(stderr, "use %s port\n", argv[0]);
        return 1;
    }

    LOG("--- INITIALIZATION ---");

    // podłączenie bazy danych
    if(!dbconnect()) {
        fprintf(stderr, "connection to database failed, cannot continue\n");
        return 1;
    }

    // start wątku serwera nasłuchowego TCP
    pthread_t nstid;
    pthread_create(&nstid, NULL, tcpserver, NULL);
    LOG("TCP server started on port %d", port);

    // start wątku serwera nasłuchowego UDP
    pthread_create(&nstid, NULL, udpserver, NULL);
    LOG("UDP server started on port %d", port);

    // konsola sterowania serwerem
    for(;;) {
        char cmdline[1024], cmd[1024];
        int n, arg1;
        printf("%% "); fflush(stdout);
        if(!fgets(cmdline, sizeof(cmdline), stdin)) break;
        n = sscanf(cmdline, "%s %d", cmd, &arg1);
        if(n < 1) continue;
        if(!strcmp(cmd, "exit")) {
            break;
        } else if(!strcmp(cmd, "status")) {
            printf("process pid     %d\n", getpid());
            printf("listening port  %d\n", port);
            printf("working threads %d\n", nthreads);
        } else if(!strcmp(cmd, "list")) {
            int i;
            printf("desc\tip\t\tlogin\n");
            for(i = 0; i < MAXCLIENTS; i++) {
                if(clients[i]) {
                    uint8_t *ip = (uint8_t *) &clients[i]->sin_addr;
                    char login[1024];
                    getLogin(clients[i]->user, login, sizeof(login));
                    printf("%d\t%d.%d.%d.%d\t%s\n", clients[i]->sock, ip[0], ip[1], ip[2], ip[3], login);
                }
            }
        } else if(!strcmp(cmd, "users")) {
            int i;
            printf("id\tlogin\n");
            forAllUsers(printUser);
        } else {
            fprintf(stderr, "command %s not recognized\n", cmd);
        }
    }
    LOG("Server stopped");

    return 0;
}
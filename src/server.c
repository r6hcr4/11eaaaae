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

uint16_t port;

void *netserver(void *arg) {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if(bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
        LOG("bind to port %hd failed", port);
        fprintf(stderr, "\ncannot start, check log\n");
        exit(2);
    }
    listen(sock, 5);
    for(;;) {
        struct sockaddr_in clientaddr;
        unsigned int sockaddr_size = sizeof(clientaddr);
        int csock = accept(sock, (struct sockaddr *) &clientaddr, &sockaddr_size);
    }
}

int main(int argc, char* argv[]) {
    _log = fopen(LOGFNAME, "a");
    if(!_log) {
        _log = stderr;
        LOG("unable to open log file %s", LOGFNAME);
    }
    if(argc < 2 || (port = atoi(argv[1])) <= 0) {
        fprintf(stderr, "use %s port\n", argv[0]);
        return 1;
    }
    pthread_t nstid;
    pthread_create(&nstid, NULL, netserver, NULL);

    LOG("TCP server g1 started on port %d", port);
    for(;;) {
        char cmd[1024];
        printf("%% "); fflush(stdout);
        if(!fgets(cmd, sizeof(cmd), stdin)) break;
    }

    return 0;
}
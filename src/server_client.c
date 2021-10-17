#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "server.h"
#include "server_lib.h"
#include "server_client.h"

void *cthread(void *arg) {
    nthreads++;
    struct cthread_arg *carg = (struct cthread_arg *) arg;
    uint8_t *ip = (uint8_t *) &carg->sin_addr;
    LOG("Connection %d from %hd.%hd.%hd.%hd established", carg->sock, ip[0], ip[1], ip[2], ip[3]);
    write(carg->sock, "Hello\r\n", 7);
    sleep(10);
    write(carg->sock, "Bye\r\n", 5);
    close(carg->sock);
    LOG("Connection %d closed", carg->sock);
    free(carg);
    nthreads--;
}
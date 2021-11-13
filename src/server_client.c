#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "server.h"
#include "server_lib.h"
#include "server_client.h"

void *cthread(void *arg) {
    nthreads++;
    struct cthread_arg *carg = (struct cthread_arg *) arg;
    clients[carg->sock] = carg;
    uint8_t *ip = (uint8_t *) &carg->sin_addr;
    LOG("Connection %d from %hd.%hd.%hd.%hd established", carg->sock, ip[0], ip[1], ip[2], ip[3]);

    // konwersacja z klientem
    FILE *input = fdopen(carg->sock, "r"), *output = fdopen(carg->sock, "w");
    char line[1024];
    for(;;) {
        fgets(line, sizeof(line), input);
        fputs(line, output); fflush(output);
    }
    // koniec konwersacji

    close(carg->sock);
    LOG("Connection %d closed", carg->sock);
    clients[carg->sock] = NULL;
    free(carg);
    nthreads--;
}
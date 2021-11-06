#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
    char line[1024], cmd[1024];
    int param, sum = 0;
    for(;;) {
        if(!fgets(line, 1024, input)) break;
        int n = sscanf(line, "%s %d", cmd, &param);
        if(n < 2) param = 0;
        if(!strcmp(cmd, "exit")) break;
        if(!strcmp(cmd, "add")) {
            sum += param;
            fprintf(output, "%d\r\n", sum);
        } else {
            fputs("OK\r\n", output);
        }
        fflush(output);
    }
    // koniec konwersacji

    close(carg->sock);
    LOG("Connection %d closed", carg->sock);
    clients[carg->sock] = NULL;
    free(carg);
    nthreads--;
}
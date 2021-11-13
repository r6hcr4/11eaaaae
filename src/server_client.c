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
    setbuf(output, NULL);
    char line[1024], cmd[1024], arg1[1024];
    for(;;) {
        if(!fgets(line, sizeof(line), input)) break;
        int narg = sscanf(line, "%s %s", cmd, arg1);
        if(!strcmp(cmd, "login")) {
            // logowanie
            if(narg > 1) {
                // użytkownik podał login
                if(carg->login) {
                    LOG("Connection %d: user %s logged out", carg->sock, carg->login);
                    free(carg->login);
                }
                carg->login = strdup(arg1);
                LOG("Connection %d: user %s logged in", carg->sock, carg->login);
            }
            fprintf(output, "You are %s\r\n", carg->login);
        } else if(!strcmp(cmd, "logout") && carg->login) {
            // wylogowywanie
            LOG("Connection %d: user %s logged out", carg->sock, carg->login);
            free(carg->login);
            carg->login = NULL;
        } else if(!strcmp(cmd, "list")) {
            // lista zalogowanych
            int i;
            for(i = 0; i < MAXCLIENTS; i++) {
                if(clients[i] && clients[i]->login) {
                    fprintf(output, "%s%s\n", clients[i]->login, i == carg->sock ? " (me)" : "");
                }
            }
        }else if(!strcmp(cmd, "send")) {
            // wysyłanie wiadomości

        } else {
            fprintf(output, "Unrecognized command %s\r\n", cmd);
        }
    }
    // koniec konwersacji

    if(carg->login) free(carg->login);
    close(carg->sock);
    LOG("Connection %d closed", carg->sock);
    clients[carg->sock] = NULL;
    free(carg);
    nthreads--;
}
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
    int narg, state = 0; // 0 = wprowadzenie poleceń
    char line[1024], cmd[1024], arg1[1024], sendTo[1024];
    for(;;) {
        if(!fgets(line, sizeof(line), input)) break;
        if(!state) {
            narg = sscanf(line, "%s %s", cmd, arg1);
            if(!strcmp(cmd, "login")) {
                if(narg > 1) {
                    if(carg->login) free(carg->login);
                    carg->login = strdup(arg1);
                    LOG("Connection %d: user %s logged in", carg->sock, carg->login);
                }
                fprintf(output, "You are %s\r\n", carg->login ? carg->login : "not-logged-in");
            } else if(!strcmp(cmd, "logout")) {
                if(carg->login) {
                    LOG("Connection %d: user %s logged out", carg->sock, carg->login);
                    free(carg->login);
                    carg->login = NULL;
                }
                fputs("You are not-logged-in\r\n", output);
            } else if(!strcmp(cmd, "list")) {
                int i;
                for(i = 0; i < MAXCLIENTS; i++) {
                    if(clients[i] && clients[i]->login) {
                        fprintf(output, "%s\r\n", clients[i]->login);
                    }
                }
            } else if(!strcmp(cmd, "send")) {
                if(narg > 1) {
                    strcpy(sendTo, arg1);
                    state = 1;
                    fprintf(output, "Now type a message for %s, dot alone for end\r\n", sendTo);
                } else {
                    fputs("Use send <user>\r\n", output);
                }
            } else {
                fprintf(output, "Unknown command %s\r\n", cmd);
            }
        } else { // stan: wysyłanie danych
            if(line[0] == '.' && (line[1] == '\r' || line[1] == '\n')) {
                state = 0;
                fputs("Back to command mode\r\n", output);
            } else {
                // routing wiadomości
                int i;
                for(i = 0; i < MAXCLIENTS; i++) {
                    if(clients[i] && !strcmp(sendTo, clients[i]->login)) {
                        FILE *recipientOutput = fdopen(clients[i]->sock, "w");
                        setbuf(recipientOutput, NULL);
                        fprintf(recipientOutput, "%s: %s", carg->login ? carg->login : "not-logged-in", line);
                    }
                }
            }
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
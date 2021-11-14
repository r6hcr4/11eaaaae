#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "server.h"
#include "server_lib.h"
#include "server_client.h"
#include "server_sqlite3.h"

void *cthread(void *arg) {
    nthreads++;
    struct cthread_arg *carg = (struct cthread_arg *) arg;
    clients[carg->sock] = carg;
    uint8_t *ip = (uint8_t *) &carg->sin_addr;
    LOG("Connection %d from %hd.%hd.%hd.%hd established", carg->sock, ip[0], ip[1], ip[2], ip[3]);

    // konwersacja z klientem
    FILE *input = fdopen(carg->sock, "r"), *output = fdopen(carg->sock, "w");
    setbuf(output, NULL);
    char line[1024], cmd[1024], arg1[1024], arg2[1024];
    int sendTo = 0;
    for(;;) {
        if(!fgets(line, sizeof(line), input)) break;
        if(!sendTo) {
            // tryb komend
            int narg = sscanf(line, "%s %s %s", cmd, arg1, arg2);
            if(!strcmp(cmd, "login")) {
                // logowanie
                if(narg > 2) {
                    // użytkownik podał login i hasło
                    int user = checkUser(arg1, arg2);
                    if(!user) {
                        LOG("Connection %d: user %s authentication failed", carg->sock, arg1);
                        fprintf(output, "Login failed\r\n");
                    } else {
                        carg->user = user;
                        LOG("Connection %d: user %s logged in", carg->sock, arg1);
                        fprintf(output, "Welcome on board %s\r\n", arg1);
                    }
                } else {
                    fprintf(output, "Use: login <user> <pass>\r\n");
                }
            } else if(!strcmp(cmd, "logout") && carg->user) {
                // wylogowywanie
                carg->user = 0;
            } else if(!strcmp(cmd, "list")) {
                // lista zalogowanych
                int i;
                for(i = 0; i < MAXCLIENTS; i++) {
                    if(clients[i] && clients[i]->user) {
                        char login[1024];
                        getLogin(clients[i]->user, login);
                        fprintf(output, "%s%s\n", login, i == carg->sock ? " (me)" : "");
                    }
                }
            } else if(!strcmp(cmd, "send")) {
                // wysyłanie wiadomości
                if(narg > 1) {
                    sendTo = getUser(arg1);
                    if(sendTo) {
                        fprintf(output, "Enter messages for %s, dot alone to finish\r\n", arg1);
                    } else {
                        fprintf(output, "No such user %s\r\n", arg1);
                    }
                } else {
                    fprintf(output, "Use: send <recipient>\r\n");
                }
            } else {
                fprintf(output, "Unrecognized command %s\r\n", cmd);
            }
        } else {
            // tryb wiadomości
            if(line[0] == '.' && (line[1] == '\r' || line[1] == '\n')) {
                // przejdź do trybu komend
                fprintf(output, "Back to command mode\r\n");
                sendTo = 0;
            } else {
                // roześlij linię do wszystkich zalogowanych jako sendTo
                int i;
                for(i = 0; i < MAXCLIENTS; i++) {
                    if(clients[i] && clients[i]->user && clients[i]->user == sendTo) {
                        FILE *recipientOutput = fdopen(clients[i]->sock, "w");
                        setbuf(recipientOutput, NULL);
                        fprintf(recipientOutput, "%s", line);
                    }
                }
            }
        }
    }
    // koniec konwersacji

    close(carg->sock);
    LOG("Connection %d closed", carg->sock);
    clients[carg->sock] = NULL;
    free(carg);
    nthreads--;
}
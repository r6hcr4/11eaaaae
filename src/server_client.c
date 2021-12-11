#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <netinet/in.h>

#include "server.h"
#include "server_lib.h"
#include "server_client.h"
#include "server_sqlite3.h"

static pthread_mutex_t out_mutex = PTHREAD_MUTEX_INITIALIZER;

static void client_printf(int sender, FILE *output, char *format, ...) {
    if(!output) return;
    pthread_mutex_lock(&out_mutex);
    if(sender) {
        char login[1024];
        getLogin(sender, login, sizeof(login));
        fprintf(output, "<%s> ", login);
    } else {
        fprintf(output, "<> ");
    }
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    fflush(output);
    va_end(args);
    pthread_mutex_unlock(&out_mutex);
}

static void logout(struct cthread_arg *carg, FILE *output) {
    if(carg->user) {
        char login[1024];
        getLogin(carg->user, login, sizeof(login));
        carg->user = 0;
        LOG("Connection %d: user %s logged out", carg->sock, login);
        client_printf(0, output, "User %s logged out\n", login);
    }
}

void *cthread(void *arg) {
    nthreads++;
    struct cthread_arg *carg = (struct cthread_arg *) arg;
    clients[carg->sock] = carg;
    uint8_t *ip = (uint8_t *) &carg->sin_addr;
    LOG("Connection %d from %hd.%hd.%hd.%hd established", carg->sock, ip[0], ip[1], ip[2], ip[3]);

    // konwersacja z klientem
    FILE *input = fdopen(carg->sock, "r"), *output = fdopen(carg->sock, "w");
    char line[1024], cmd[1024], arg1[1024], arg2[1024];
    int sendTo = 0;
    for(;;) {
        if(!fgets(line, sizeof(line), input)) {
            input = output = NULL;
            break;
        }
        if(!sendTo) {
            // tryb komend
            int narg = sscanf(line, "%s %s %s", cmd, arg1, arg2);
            if(!strcmp(cmd, "device")) {
                // rejestracja urządzenia
                if(!carg->device_id && narg > 1) {
                    carg->device_id = strdup(arg1);
                    LOG("Connection %d: device %s has been registered", carg->sock, arg1);
                } else {
                    LOG("Connection %d: device cannot be registered", carg->sock);
                    client_printf(0, output, "Device cannot be registered\r\n");
                }
            } else if(!strcmp(cmd, "login")) {
                // logowanie
                if(narg > 2) {
                    // użytkownik podał login i hasło
                    int user = checkUser(arg1, arg2);
                    if(!user) {
                        LOG("Connection %d: user %s authentication failed", carg->sock, arg1);
                        client_printf(0, output, "Login failed\r\n");
                    } else {
                        logout(carg, output);
                        carg->user = user;
                        LOG("Connection %d: user %s logged in", carg->sock, arg1);
                        client_printf(0, output, "Welcome on board, %s\r\n", arg1);
                    }
                } else {
                    client_printf(0, output, "Use: login <user> <pass>\r\n");
                }
            } else if(!strcmp(cmd, "logout") && carg->user) {
                // wylogowywanie
                logout(carg, output);
            } else if(!strcmp(cmd, "list")) {
                // lista zalogowanych
                int i;
                for(i = 0; i < MAXCLIENTS; i++) {
                    if(clients[i] && clients[i]->user) {
                        char login[1024];
                        getLogin(clients[i]->user, login, sizeof(login));
                        client_printf(0, output, "%s%s\n", login, i == carg->sock ? " (me)" : "");
                    }
                }
            } else if(!strcmp(cmd, "send") && carg->user) { // tylko zalogowani
                // wysyłanie wiadomości
                if(narg > 1) {
                    sendTo = getUser(arg1);
                    if(sendTo) {
                        client_printf(0, output, "Enter messages for %s, dot alone to finish\r\n", arg1);
                    } else {
                        client_printf(0, output, "No such user %s\r\n", arg1);
                    }
                } else {
                    client_printf(0, output, "Use: send <recipient>\r\n");
                }
            } else if(!strcmp(cmd, "friend") && carg->user) { // tylko zalogowani
                // dodanie znajomości
                if(narg > 1) {
                    int friend = getUser(arg1);
                    if(friend) {
                        addFriendship(carg->user, friend);
                        client_printf(0, output, "Added %s as a friend\r\n", arg1);
                    } else {
                        client_printf(0, output, "No such user %s\r\n", arg1);
                    }
                } else {
                    client_printf(0, output, "Use: friend <user>\r\n");
                }
            } else if(!strcmp(cmd, "register")) {
                // rejestracja
                if(narg > 2) {
                    // użytkownik podał login i hasło
                    int user = registerUser(arg1, arg2);
                    if(!user) {
                        LOG("Connection %d: user %s was not created", carg->sock, arg1);
                        client_printf(0, output, "User not created\r\n");
                    } else {
                        LOG("Connection %d: user %s was created", carg->sock, arg1);
                        client_printf(0, output, "User %s created\r\n", arg1);
                    }
                } else {
                    client_printf(0, output, "Use: register <user> <pass>\r\n");
                }
            } else {
                client_printf(0, output, "Command %s cannot be performed\r\n", cmd);
            }
        } else {
            // tryb wiadomości
            if(line[0] == '.' && (line[1] == '\r' || line[1] == '\n')) {
                // przejdź do trybu komend
                client_printf(0, output, "Back to command mode\r\n");
                sendTo = 0;
            } else {
                // zapisz wiadomość w bazie (czas aktualny, nadawca, odbiorca, treść)
                saveMessage(carg->user, sendTo, line);
                // roześlij linię do wszystkich zalogowanych jako sendTo
                int i;
                for(i = 0; i < MAXCLIENTS; i++) {
                    if(clients[i] && clients[i]->user && clients[i]->user == sendTo) {
                        FILE *recipientOutput = fdopen(clients[i]->sock, "w");
                        client_printf(carg->user, recipientOutput, "%s", line);
                    }
                }
            }
        }
    }
    // koniec konwersacji

    logout(carg, output);
    close(carg->sock);
    if(carg->device_id) free(carg->device_id);
    LOG("Connection %d closed", carg->sock);
    clients[carg->sock] = NULL;
    free(carg);
    nthreads--;
}
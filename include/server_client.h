#ifndef __SERVER_CLIENT_H
#define __SERVER_CLIENT_H

struct cthread_arg {
    struct in_addr sin_addr;
    int sock;
    char *login;
};

void *cthread(void *arg);

#endif
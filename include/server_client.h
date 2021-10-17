#ifndef __SERVER_CLIENT_H
#define __SERVER_CLIENT_H

struct cthread_arg {
    struct in_addr sin_addr;
    int sock;
};

void *cthread(void *arg);

#endif
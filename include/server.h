#ifndef __SERVER_H
#define __SERVER_H

#define LOGFNAME "server.log"
#define MAXCLIENTS 1024

extern int nthreads;
extern struct cthread_arg* clients[MAXCLIENTS];

#endif
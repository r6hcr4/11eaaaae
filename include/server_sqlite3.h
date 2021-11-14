#ifndef __SERVER_SQLITE3_H
#define __SERVER_SQLITE3_H

int dbconnect();
void dbclose();

int checkUser(const char *login, const char *password);
int getUser(const char *login);
int getLogin(int n, char *login);

#endif

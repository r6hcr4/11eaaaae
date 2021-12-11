#ifndef __SERVER_SQLITE3_H
#define __SERVER_SQLITE3_H

int dbconnect();
void dbclose();

int checkUser(const char *login, const char *password);
int getUser(const char *login);
int getLogin(int n, char *login, size_t maxsize);
int registerUser(const char *login, const char *password);
void forAllUsers(void (*action)(int uid, const char *login));
void saveMessage(int sender, int recipient, const char *line);
void addFriendship(int uid, int friend);

#endif

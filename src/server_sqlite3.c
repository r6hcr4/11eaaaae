#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sqlite3.h>

#include "server_lib.h"

#define DBFILE "db/data.sqlite3"

static sqlite3 *db;
// zamek dla operacji na jednym rekordzie
static pthread_mutex_t db_mutex_a = PTHREAD_MUTEX_INITIALIZER;
// zamek dla operacji na wielu rekordach
static pthread_mutex_t db_mutex_s = PTHREAD_MUTEX_INITIALIZER;

int dbconnect() {
    
    sqlite3_stmt *stmt;

    LOG("sqlite version %s, thread-safe %d", sqlite3_libversion(), sqlite3_threadsafe());

    if(sqlite3_open(DBFILE, &db) != SQLITE_OK) {
        LOG("Connection to database %s failed: %s", DBFILE, sqlite3_errmsg(db));
        return 0;
    }

    LOG("Connection to database %s established", DBFILE);

    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM users", -1, &stmt, NULL);
    int step = sqlite3_step(stmt);
    if(step == SQLITE_ROW) {
        LOG("Number of users: %d", sqlite3_column_int(stmt, 0));
    }
    return 1;
}

void dbclose() {
    sqlite3_close(db);
    LOG("Connection to database %s closed", DBFILE);
}

// login+password valid?
int checkUser(const char *login, const char *password) {
    pthread_mutex_lock(&db_mutex_a);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id FROM users WHERE login = ? AND password = ?", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, login, strlen(login), NULL);
    sqlite3_bind_text(stmt, 2, password, strlen(password), NULL);
    int res = sqlite3_step(stmt) != SQLITE_ROW ? 0 : sqlite3_column_int(stmt, 0);
    pthread_mutex_unlock(&db_mutex_a);
    return res;
}

// login -> uid
int getUser(const char *login) {
    pthread_mutex_lock(&db_mutex_a);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id FROM users WHERE login = ?", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, login, strlen(login), NULL);
    int res = sqlite3_step(stmt) != SQLITE_ROW ? 0 : sqlite3_column_int(stmt, 0);
    pthread_mutex_unlock(&db_mutex_a);
    return res;
}

// uid -> login
int getLogin(int n, char *login, size_t maxsize) {
    pthread_mutex_lock(&db_mutex_a);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT login FROM users WHERE id = ?", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, n);
    int res = 0;
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        strncpy(login, sqlite3_column_text(stmt, 0), maxsize);
        res = n;
    } else {
        strncpy(login, "(not-logged-in)", maxsize);
    }
    pthread_mutex_unlock(&db_mutex_a);
    return res;
}

// rejestracja
int registerUser(const char *login, const char *password) {
    pthread_mutex_lock(&db_mutex_a);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO users (login,password) VALUES (?,?)", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, login, strlen(login), NULL);
    sqlite3_bind_text(stmt, 2, password, strlen(password), NULL);
    int res = 0;
    if(sqlite3_step(stmt) == SQLITE_DONE) {
        sqlite3_prepare_v2(db, "SELECT last_insert_rowid()", -1, &stmt, NULL);
        res = sqlite3_step(stmt) != SQLITE_ROW ? 0 : sqlite3_column_int(stmt, 0);
    }
    pthread_mutex_unlock(&db_mutex_a);
    return res;
}

// zrób "action" dla wszystkich użytkowników
void forAllUsers(void (*action)(int uid, const char *login)) {
    pthread_mutex_lock(&db_mutex_s);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id,login FROM users", -1, &stmt, NULL);
    int res = 0;
    static char login[1024];
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        int uid = sqlite3_column_int(stmt, 0);
        strncpy(login, sqlite3_column_text(stmt, 1), sizeof(login));
        action(uid, login);
    }
    pthread_mutex_unlock(&db_mutex_s);
}

// zapisz wiadomość
void saveMessage(int sender, int recipient, const char *line) {
    pthread_mutex_lock(&db_mutex_a);
    int date = time(NULL);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO messages (sent,sender,recipient,line) VALUES (?,?,?,?)", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, date);
    sqlite3_bind_int(stmt, 2, sender);
    sqlite3_bind_int(stmt, 3, recipient);
    sqlite3_bind_text(stmt, 4, line, strlen(line), NULL);
    sqlite3_step(stmt);
    pthread_mutex_unlock(&db_mutex_a);
}

// zrób "action" dla wszystkich wiadomość w których zaangażowany jest użytkownik uid
void forAllMessagesPerUser(int uid, FILE *output, void (*action)(FILE *output, int sent, const char *sender, const char *recipient, const char *line)) {
    pthread_mutex_lock(&db_mutex_s);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "select sent,u1.login as sender,u2.login as recipient,line from messages left join users u1 on sender=u1.id left join users u2 on recipient=u2.id where sender=? or recipient=? order by sent desc", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, uid);
    sqlite3_bind_int(stmt, 2, uid);
    static char sender[1024], recipient[1024], line[1024];
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        int sent = sqlite3_column_int(stmt, 0);
        strncpy(sender, sqlite3_column_text(stmt, 1), sizeof(sender));
        strncpy(recipient, sqlite3_column_text(stmt, 2), sizeof(recipient));
        strncpy(line, sqlite3_column_text(stmt, 3), sizeof(line));
        action(output, sent, sender, recipient, line);
    }
    pthread_mutex_unlock(&db_mutex_s);
}
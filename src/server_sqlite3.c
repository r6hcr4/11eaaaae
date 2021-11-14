#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sqlite3.h>

#include "server_lib.h"

#define DBFILE "db/data.sqlite3"

static sqlite3 *db;
static pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

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

int checkUser(const char *login, const char *password) {
    pthread_mutex_lock(&db_mutex);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id FROM users WHERE login = ? AND password = ?", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, login, strlen(login), NULL);
    sqlite3_bind_text(stmt, 2, password, strlen(password), NULL);
    int res = sqlite3_step(stmt) != SQLITE_ROW ? 0 : sqlite3_column_int(stmt, 0);
    pthread_mutex_unlock(&db_mutex);
    return res;
}

int getUser(const char *login) {
    pthread_mutex_lock(&db_mutex);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id FROM users WHERE login = ?", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, login, strlen(login), NULL);
    int res = sqlite3_step(stmt) != SQLITE_ROW ? 0 : sqlite3_column_int(stmt, 0);
    pthread_mutex_unlock(&db_mutex);
    return res;
}

int getLogin(int n, char *login, size_t maxsize) {
    pthread_mutex_lock(&db_mutex);
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
    pthread_mutex_unlock(&db_mutex);
    return res;
}
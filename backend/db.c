// db.c - PostgreSQL persistence
#include "db.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PGconn *conn = NULL;

static void db_log_error(const char *msg) {
    fprintf(stderr, "[DB] %s: %s\n", msg, PQerrorMessage(conn));
}

bool db_init(void) {
    const char *host = getenv("DB_HOST");
    const char *port = getenv("DB_PORT");
    const char *user = getenv("DB_USER");
    const char *pass = getenv("DB_PASSWORD");
    const char *name = getenv("DB_NAME");

    if (!host) host = "localhost";
    if (!port) port = "5432";
    if (!user || !name) {
        fprintf(stderr, "[DB] DB_USER and DB_NAME must be set\n");
        return false;
    }

    char conninfo[512];
    snprintf(conninfo, sizeof(conninfo),
             "host=%s port=%s dbname=%s user=%s%s%s",
             host, port, name, user,
             pass ? " password=" : "",
             pass ? pass : "");

    conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "[DB] Connection failed: %s\n", PQerrorMessage(conn));
        return false;
    }

    // Ensure tables exist (id serial; no created_at as requested)
    const char *user_sql =
        "CREATE TABLE IF NOT EXISTS users ("\
        "id SERIAL PRIMARY KEY,"\
        "username TEXT UNIQUE NOT NULL,"\
        "password_hash TEXT NOT NULL"\
        ");";

    const char *sub_sql =
        "CREATE TABLE IF NOT EXISTS submissions ("\
        "id SERIAL PRIMARY KEY,"\
        "user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,"\
        "problem_id TEXT NOT NULL,"\
        "code TEXT NOT NULL,"\
        "score INTEGER NOT NULL,"\
        "status TEXT NOT NULL"\
        ");";

    PGresult *res = PQexec(conn, user_sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        db_log_error("Failed to create users table");
        PQclear(res);
        return false;
    }
    PQclear(res);

    res = PQexec(conn, sub_sql);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        db_log_error("Failed to create submissions table");
        PQclear(res);
        return false;
    }
    PQclear(res);

    return true;
}

static cJSON *row_to_user(PGresult *res, int row) {
    cJSON *obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", atoi(PQgetvalue(res, row, 0)));
    cJSON_AddStringToObject(obj, "username", PQgetvalue(res, row, 1));
    cJSON_AddStringToObject(obj, "password_hash", PQgetvalue(res, row, 2));
    return obj;
}

cJSON *db_find_user_by_username(const char *username) {
    if (!conn) return NULL;
    const char *params[1] = { username };
    PGresult *res = PQexecParams(conn,
        "SELECT id, username, password_hash FROM users WHERE username = $1",
        1, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        db_log_error("db_find_user_by_username failed");
        PQclear(res);
        return NULL;
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }

    cJSON *user = row_to_user(res, 0);
    PQclear(res);
    return user;
}

cJSON *db_insert_user(const char *username, const char *password_hash) {
    if (!conn) return NULL;
    const char *params[2] = { username, password_hash };
    PGresult *res = PQexecParams(conn,
        "INSERT INTO users (username, password_hash) VALUES ($1, $2) RETURNING id, username, password_hash",
        2, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        db_log_error("db_insert_user failed");
        PQclear(res);
        return NULL;
    }

    cJSON *user = row_to_user(res, 0);
    PQclear(res);
    return user;
}

bool db_insert_submission(int user_id, const char *problem_id, const char *code, int score, const char *status) {
    if (!conn) return false;
    char user_id_str[32]; snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    char score_str[32]; snprintf(score_str, sizeof(score_str), "%d", score);

    const char *params[5] = { user_id_str, problem_id, code, score_str, status };
    PGresult *res = PQexecParams(conn,
        "INSERT INTO submissions (user_id, problem_id, code, score, status) VALUES ($1, $2, $3, $4, $5)",
        5, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        db_log_error("db_insert_submission failed");
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

cJSON *db_get_submissions_by_user(int user_id) {
    if (!conn) return NULL;
    char user_id_str[32]; snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    const char *params[1] = { user_id_str };

    PGresult *res = PQexecParams(conn,
        "SELECT id, user_id, problem_id, code, score, status FROM submissions WHERE user_id = $1",
        1, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        db_log_error("db_get_submissions_by_user failed");
        PQclear(res);
        return NULL;
    }

    int n = PQntuples(res);
    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < n; i++) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "id", atoi(PQgetvalue(res, i, 0)));
        cJSON_AddNumberToObject(obj, "user_id", atoi(PQgetvalue(res, i, 1)));
        cJSON_AddStringToObject(obj, "problem_id", PQgetvalue(res, i, 2));
        cJSON_AddStringToObject(obj, "code", PQgetvalue(res, i, 3));
        cJSON_AddNumberToObject(obj, "score", atoi(PQgetvalue(res, i, 4)));
        cJSON_AddStringToObject(obj, "status", PQgetvalue(res, i, 5));
        cJSON_AddItemToArray(arr, obj);
    }

    PQclear(res);
    return arr;
}

cJSON *db_get_top_submissions(int limit) {
    if (!conn) return NULL;
    if (limit <= 0) limit = 5;

    char limit_str[32]; snprintf(limit_str, sizeof(limit_str), "%d", limit);
    const char *params[1] = { limit_str };

    PGresult *res = PQexecParams(conn,
        "SELECT s.id, s.user_id, u.username, s.problem_id, s.score, s.status "
        "FROM submissions s JOIN users u ON s.user_id = u.id "
        "ORDER BY s.score DESC, s.id ASC LIMIT $1",
        1, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        db_log_error("db_get_top_submissions failed");
        PQclear(res);
        return NULL;
    }

    int n = PQntuples(res);
    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < n; i++) {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "id", atoi(PQgetvalue(res, i, 0)));
        cJSON_AddNumberToObject(obj, "user_id", atoi(PQgetvalue(res, i, 1)));
        cJSON_AddStringToObject(obj, "username", PQgetvalue(res, i, 2));
        cJSON_AddStringToObject(obj, "problem_id", PQgetvalue(res, i, 3));
        cJSON_AddNumberToObject(obj, "score", atoi(PQgetvalue(res, i, 4)));
        cJSON_AddStringToObject(obj, "status", PQgetvalue(res, i, 5));
        cJSON_AddItemToArray(arr, obj);
    }

    PQclear(res);
    return arr;
}

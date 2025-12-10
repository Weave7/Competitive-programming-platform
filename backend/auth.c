// auth.c
#include "auth.h"
#include "db.h"
#include <sodium.h>
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>

char* auth_login(const char *username, const char *password) {
    cJSON *user = db_find_user_by_username(username);
    if (!user) return NULL;

    const char *hash = cJSON_GetObjectItem(user, "password_hash")->valuestring;
    bool valid = crypto_pwhash_str_verify(hash, password, (unsigned long long)strlen(password)) == 0;
    int id = cJSON_GetObjectItem(user, "id")->valueint;
    cJSON_Delete(user);

    if (!valid) return NULL;

    char *buf = malloc(32);
    if (!buf) return NULL;
    snprintf(buf, 32, "%d", id);
    return buf;
}

char* auth_register(const char *username, const char *password) {
    // 1) check existing user
    cJSON *existing = db_find_user_by_username(username);
    if (existing) {
        cJSON_Delete(existing);
        return NULL; // already exists
    }

    // 2) hash password using libsodium
    char hash[crypto_pwhash_STRBYTES];
    if (sodium_init() < 0) return NULL;
    if (crypto_pwhash_str(hash, password, (unsigned long long)strlen(password),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        return NULL;
    }

    // 3) insert user into DB
    cJSON *user = db_insert_user(username, hash);
    if (!user) return NULL;

    int id = cJSON_GetObjectItem(user, "id")->valueint;
    cJSON_Delete(user);

    char *ret = malloc(32);
    if (!ret) return NULL;
    snprintf(ret, 32, "%d", id);
    return ret;
}

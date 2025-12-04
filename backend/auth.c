// auth.c
#include "auth.h"
#include "teable.h"
#include <sodium.h>
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>

char* auth_login(const char *username, const char *password) {
    cJSON *records;
    char filter[128];
    snprintf(filter, sizeof(filter), "{\"username\": \"%s\"}", username);
    if (!teable_query(getenv("TEABLE_USERS_BASE_ID"), getenv("TEABLE_USERS_TABLE_ID"), filter, &records)) return NULL;
    if (cJSON_GetArraySize(records) == 0) {
        cJSON_Delete(records);
        return NULL;
    }
    cJSON *fields = cJSON_GetObjectItem(cJSON_GetArrayItem(records, 0), "fields");
    const char *id = cJSON_GetObjectItem(fields, "id")->valuestring;
    const char *hash = cJSON_GetObjectItem(fields, "password_hash")->valuestring;
    bool valid = crypto_pwhash_str_verify(hash, password, (unsigned long long)strlen(password)) == 0;
    cJSON_Delete(records);
    return valid ? strdup(id) : NULL;
}

char* auth_register(const char *username, const char *password) {
    // 1) check existing user
    cJSON *records = NULL;
    char filter[256];
    snprintf(filter, sizeof(filter), "{\"username\": \"%s\"}", username);
    if (!teable_query(getenv("TEABLE_USERS_BASE_ID"), getenv("TEABLE_USERS_TABLE_ID"), filter, &records)) return NULL;
    if (cJSON_GetArraySize(records) > 0) {
        cJSON_Delete(records);
        return NULL; // already exists
    }
    cJSON_Delete(records);

    // 2) hash password using libsodium
    char hash[crypto_pwhash_STRBYTES];
    if (sodium_init() < 0) return NULL;
    if (crypto_pwhash_str(hash, password, (unsigned long long)strlen(password),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        return NULL;
    }

    // 3) build fields JSON and insert
    cJSON *fields = cJSON_CreateObject();
    cJSON_AddStringToObject(fields, "username", username);
    cJSON_AddStringToObject(fields, "password_hash", hash);

    bool ok = teable_insert(getenv("TEABLE_USERS_BASE_ID"), getenv("TEABLE_USERS_TABLE_ID"), fields);
    cJSON_Delete(fields);

    if (!ok) return NULL;

    // If teable_insert doesn't return the new id, you can re-query by username to get the id:
    cJSON *newrec = NULL;
    if (!teable_query(getenv("TEABLE_USERS_BASE_ID"), getenv("TEABLE_USERS_TABLE_ID"), filter, &newrec)) return NULL;
    if (cJSON_GetArraySize(newrec) == 0) { cJSON_Delete(newrec); return NULL; }
    const char *id = cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(newrec,0), "fields"), "id")->valuestring;
    char *ret = strdup(id);
    cJSON_Delete(newrec);
    return ret;
}
// teable.c
#include "teable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static size_t curl_write_cb(void *contents, size_t size, size_t nmemb, CurlChunk *chunk) {
    size_t new_size = chunk->size + size * nmemb;
    chunk->data = realloc(chunk->data, new_size + 1);
    memcpy(chunk->data + chunk->size, contents, size * nmemb);
    chunk->size = new_size;
    chunk->data[chunk->size] = '\0';
    return size * nmemb;
}

bool teable_query(const char *base_id, const char *table_id, const char *filter, cJSON **records) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    char url[512];
    snprintf(url, sizeof(url), "https://app.teable.ai/api/v1/workspace/%s/table/%s/record?pageSize=100", base_id, table_id);
    if (filter) {
        char *tmp = malloc(strlen(url) + strlen(filter) + 20);
        snprintf(tmp, strlen(url) + strlen(filter) + 20, "%s&filterByFormula=%s", url, filter);
        strcpy(url, tmp);
        free(tmp);
    }
    CurlChunk chunk = { .data = malloc(1), .size = 0 };
    struct curl_slist *headers = NULL;
    char auth[256]; snprintf(auth, sizeof(auth), "Authorization: Bearer %s", getenv("TEABLE_TOKEN"));
    headers = curl_slist_append(headers, auth);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    if (res != CURLE_OK) {
        free(chunk.data);
        return false;
    }
    *records = cJSON_Parse(chunk.data);
    free(chunk.data);
    return *records != NULL;
}

bool teable_insert(const char *base_id, const char *table_id, cJSON *fields) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    char url[512];
    snprintf(url, sizeof(url), "https://app.teable.ai/api/v1/workspace/%s/table/%s/record", base_id, table_id);
    char *body = cJSON_Print(fields);
    CurlChunk chunk = { .data = malloc(1), .size = 0 };
    struct curl_slist *headers = NULL;
    char auth[256]; snprintf(auth, sizeof(auth), "Authorization: Bearer %s", getenv("TEABLE_TOKEN"));
    headers = curl_slist_append(headers, auth);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(chunk.data);
    free(body);
    return res == CURLE_OK;
}
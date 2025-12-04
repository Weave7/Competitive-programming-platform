// teable.h
#ifndef TEABLE_H
#define TEABLE_H
#include <curl/curl.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

typedef struct { char *data; size_t size; } CurlChunk;

bool teable_query(const char *base_id, const char *table_id, const char *filter, cJSON **records);
bool teable_insert(const char *base_id, const char *table_id, cJSON *fields);
#endif
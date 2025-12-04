#ifndef UTILS_H
#define UTILS_H
#include <cjson/cJSON.h>
#include <microhttpd.h>

cJSON* parse_json(const char* json_str);
char* json_stringify(cJSON* json);
int send_json_response(struct MHD_Connection *connection, int status, cJSON* json);
#endif
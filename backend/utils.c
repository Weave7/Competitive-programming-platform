#include "utils.h"
#include <stdlib.h>
#include <string.h>

cJSON* parse_json(const char* json_str) {
    return cJSON_Parse(json_str);
}

char* json_stringify(cJSON* json) {
    return cJSON_Print(json);
}

int send_json_response(struct MHD_Connection *connection, int status, cJSON* json) {
    char* body = json_stringify(json);
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(body), body, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
    int ret = MHD_queue_response(connection, status, response);
    MHD_destroy_response(response);
    free(body);
    cJSON_Delete(json);
    return ret;
}
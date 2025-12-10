#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "auth.h"
#include "judge.h"
#include "db.h"
#include <sodium.h>
#include <time.h>

#define PORT 8000

struct ConnectionInfo {
    char *body;
    size_t size;
};

// Helper to send a simple empty response with a status code
static int send_simple_status(struct MHD_Connection *connection, int status) {
    struct MHD_Response *resp = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(resp, "Access-Control-Allow-Origin", "*");
    MHD_add_response_header(resp, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    MHD_add_response_header(resp, "Access-Control-Allow-Headers", "Content-Type");
    int ret = MHD_queue_response(connection, status, resp);
    MHD_destroy_response(resp);
    return ret;
}

static enum MHD_Result handle_request(void *cls, struct MHD_Connection *connection, const char *url,
                          const char *method, const char *version, const char *upload_data,
                          size_t *upload_data_size, void **con_cls) {
    // Handle CORS preflight
    if (strcmp(method, "OPTIONS") == 0) {
        struct MHD_Response *resp = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(resp, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(resp, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        MHD_add_response_header(resp, "Access-Control-Allow-Headers", "Content-Type");
        int ret = MHD_queue_response(connection, MHD_HTTP_NO_CONTENT, resp);
        MHD_destroy_response(resp);
        return ret;
    }

    if (strcmp(method, "POST") != 0 && strcmp(method, "GET") != 0) {
        return send_simple_status(connection, MHD_HTTP_METHOD_NOT_ALLOWED);
    }

    // Initialize per-connection state on first call
    if (*con_cls == NULL) {
        struct ConnectionInfo *info = calloc(1, sizeof(struct ConnectionInfo));
        if (!info) return send_simple_status(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
        info->body = NULL;
        info->size = 0;
        *con_cls = info;
        // libmicrohttpd expects us to return MHD_YES and be called again
        return MHD_YES;
    }

    struct ConnectionInfo *info = (struct ConnectionInfo *)*con_cls;

    if (strcmp(method, "POST") == 0) {
        // Accumulate body chunks
        if (*upload_data_size != 0) {
            char *new_body = realloc(info->body, info->size + *upload_data_size + 1);
            if (!new_body) {
                free(info->body);
                free(info);
                *con_cls = NULL;
                return send_simple_status(connection, MHD_HTTP_INTERNAL_SERVER_ERROR);
            }
            info->body = new_body;
            memcpy(info->body + info->size, upload_data, *upload_data_size);
            info->size += *upload_data_size;
            info->body[info->size] = '\0';
            *upload_data_size = 0;
            return MHD_YES;
        }

        // No more upload data: process the request
        const char *username = NULL, *password = NULL, *user_id = NULL, *code = NULL, *problem_id = NULL;
        cJSON *req_json = NULL;

        if (info->body) {
            req_json = parse_json(info->body);
        }

        if (!req_json) {
            free(info->body);
            free(info);
            *con_cls = NULL;
            return (send_simple_status(connection, MHD_HTTP_BAD_REQUEST) == MHD_YES) ? MHD_YES : MHD_NO;
        }

        if (strstr(url, "/login")) {
            username = cJSON_GetObjectItem(req_json, "username")->valuestring;
            password = cJSON_GetObjectItem(req_json, "password")->valuestring;
            char *uid = auth_login(username, password);
            if (!username || !password) {
                cJSON_Delete(req_json);
                return (send_simple_status(connection, MHD_HTTP_BAD_REQUEST) == MHD_YES) ? MHD_YES : MHD_NO;
            }
            if (uid) {
                cJSON *resp = cJSON_CreateObject();
                cJSON_AddStringToObject(resp, "username", username);
                cJSON_AddStringToObject(resp, "user_id", uid);
                cJSON_AddStringToObject(resp, "message", "Logged in");
                free(uid);
                cJSON_Delete(req_json);
                return (send_json_response(connection, MHD_HTTP_OK, resp) == MHD_YES) ? MHD_YES : MHD_NO;
            }
            cJSON_Delete(req_json);
            return (send_simple_status(connection, MHD_HTTP_UNAUTHORIZED) == MHD_YES) ? MHD_YES : MHD_NO;
        } else if (strstr(url, "/register")) {
            username = cJSON_GetObjectItem(req_json, "username")->valuestring;
            password = cJSON_GetObjectItem(req_json, "password")->valuestring;
            
            if (!username || !password) {
                cJSON_Delete(req_json);
                return (send_simple_status(connection, MHD_HTTP_BAD_REQUEST) == MHD_YES) ? MHD_YES : MHD_NO;
            }
            
            // Create new user record in teable
            cJSON *user_fields = cJSON_CreateObject();
            cJSON_AddStringToObject(user_fields, "username", username);
            cJSON_AddStringToObject(user_fields, "password", password);

            char *uid=auth_register(username,password);

            if (uid) {
                cJSON *resp = cJSON_CreateObject();
                cJSON_AddStringToObject(resp, "message", "User registered successfully");
                cJSON_Delete(req_json);
                return (send_json_response(connection, MHD_HTTP_CREATED, resp) == MHD_YES) ? MHD_YES : MHD_NO;
            } else {
                cJSON_Delete(req_json);
                return (send_simple_status(connection, MHD_HTTP_INTERNAL_SERVER_ERROR) == MHD_YES) ? MHD_YES : MHD_NO;
            }
        } else if (strstr(url, "/submit")) {
            user_id = cJSON_GetObjectItem(req_json, "user_id")->valuestring;
            problem_id = cJSON_GetObjectItem(req_json, "problem_id")->valuestring;
            code = cJSON_GetObjectItem(req_json, "code")->valuestring;
            JudgeResult jr = judge_submission(code, problem_id);

            int uid = atoi(user_id);
            db_insert_submission(uid, problem_id, code, (int)jr.score, jr.status);

            cJSON *resp = cJSON_CreateObject();
            cJSON_AddNumberToObject(resp, "score", jr.score);
            cJSON_AddStringToObject(resp, "status", jr.status);
            cJSON_Delete(req_json);
            free(info->body);
            free(info);
            *con_cls = NULL;
            return (send_json_response(connection, MHD_HTTP_OK, resp) == MHD_YES) ? MHD_YES : MHD_NO;
        }

        // Unknown POST path
        cJSON_Delete(req_json);
        free(info->body);
        free(info);
        *con_cls = NULL;
        return (send_simple_status(connection, MHD_HTTP_NOT_FOUND) == MHD_YES) ? MHD_YES : MHD_NO;
    } else if (strcmp(method, "GET") == 0 && strstr(url, "/dashboard/")) {
        const char *user_id = url + strlen("/dashboard/");
        int uid = atoi(user_id);

        cJSON *my_subs = db_get_submissions_by_user(uid);
        cJSON *top = db_get_top_submissions(5);

        cJSON *resp = cJSON_CreateObject();
        if (my_subs)
            cJSON_AddItemToObject(resp, "my_subs", my_subs);
        else
            cJSON_AddItemToObject(resp, "my_subs", cJSON_CreateArray());

        if (top)
            cJSON_AddItemToObject(resp, "top_scores", top);
        else
            cJSON_AddItemToObject(resp, "top_scores", cJSON_CreateArray());

        return (send_json_response(connection, MHD_HTTP_OK, resp) == MHD_YES) ? MHD_YES : MHD_NO;
    }

    return (send_simple_status(connection, MHD_HTTP_NOT_FOUND) == MHD_YES) ? MHD_YES : MHD_NO;
}

int main(int argc, char *argv[]) {
    if (sodium_init() < 0) {
        fprintf(stderr, "Failed to initialize libsodium\n");
        return 1;
    }
    if (!db_init()) {
        fprintf(stderr, "Failed to initialize database connection\n");
        return 1;
    }
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL,
                                                  &handle_request, NULL, MHD_OPTION_END);
    if (!daemon) return 1;
    printf("Server running on port %d\n", PORT);
    fflush(stdout);
    // Keep the server running (pause indefinitely)
    pause();
    MHD_stop_daemon(daemon);
    return 0;
}
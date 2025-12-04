#include <microhttpd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "teable.h"
#include "auth.h"
#include "judge.h"
#include <sodium.h>
#include <time.h>

#define PORT 8000

// Helper to send a simple empty response with a status code
static int send_simple_status(struct MHD_Connection *connection, int status) {
    struct MHD_Response *resp = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, status, resp);
    MHD_destroy_response(resp);
    return ret;
}

static enum MHD_Result handle_request(void *cls, struct MHD_Connection *connection, const char *url,
                          const char *method, const char *version, const char *upload_data,
                          size_t *upload_data_size, void **con_cls) {
    if (strcmp(method, "POST") != 0 && strcmp(method, "GET") != 0) {
        return send_simple_status(connection, MHD_HTTP_METHOD_NOT_ALLOWED);
    }

    const char *username = NULL, *password = NULL, *user_id = NULL, *code = NULL, *problem_id = NULL;
    cJSON *req_json = NULL;

    if (strcmp(method, "POST") == 0 && *upload_data_size) {
        req_json = parse_json(upload_data);
        *upload_data_size = 0;
        if (strstr(url, "/login")) {
            username = cJSON_GetObjectItem(req_json, "username")->valuestring;
            password = cJSON_GetObjectItem(req_json, "password")->valuestring;
            char *uid = auth_login(username, password);
            if (uid) {
                cJSON *resp = cJSON_CreateObject();
                cJSON_AddStringToObject(resp, "user_id", uid);
                cJSON_AddStringToObject(resp, "message", "Logged in");
                free(uid);
                cJSON_Delete(req_json);
                return (send_json_response(connection, MHD_HTTP_OK, resp) == MHD_YES) ? MHD_YES : MHD_NO;
            }
            cJSON_Delete(req_json);
            return (send_simple_status(connection, MHD_HTTP_UNAUTHORIZED) == MHD_YES) ? MHD_YES : MHD_NO;
        } else if (strstr(url, "/submit")) {
            user_id = cJSON_GetObjectItem(req_json, "user_id")->valuestring;
            problem_id = cJSON_GetObjectItem(req_json, "problem_id")->valuestring;
            code = cJSON_GetObjectItem(req_json, "code")->valuestring;
            JudgeResult jr = judge_submission(code, problem_id);
            cJSON *fields = cJSON_CreateObject();
            cJSON_AddStringToObject(fields, "user_id", user_id);
            cJSON_AddStringToObject(fields, "problem_id", problem_id);
            cJSON_AddStringToObject(fields, "code", code);
            cJSON_AddNumberToObject(fields, "score", jr.score);
            cJSON_AddStringToObject(fields, "status", jr.status);
            char ts[32]; time_t now = time(NULL); strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%S", localtime(&now));
            cJSON_AddStringToObject(fields, "timestamp", ts);
            teable_insert(getenv("TEABLE_SUBMISSIONS_BASE_ID"), getenv("TEABLE_SUBMISSIONS_TABLE_ID"), fields);
            cJSON_Delete(fields);
            cJSON *resp = cJSON_CreateObject();
            cJSON_AddNumberToObject(resp, "score", jr.score);
            cJSON_AddStringToObject(resp, "status", jr.status);
            cJSON_Delete(req_json);
            return (send_json_response(connection, MHD_HTTP_OK, resp) == MHD_YES) ? MHD_YES : MHD_NO;
        }
    } else if (strcmp(method, "GET") == 0 && strstr(url, "/dashboard/")) {
        user_id = url + strlen("/dashboard/");
        cJSON *my_subs = NULL, *all_subs = NULL;
        char filter[128]; snprintf(filter, sizeof(filter), "{\"user_id\": \"%s\"}", user_id);
        teable_query(getenv("TEABLE_SUBMISSIONS_BASE_ID"), getenv("TEABLE_SUBMISSIONS_TABLE_ID"), filter, &my_subs);
        teable_query(getenv("TEABLE_SUBMISSIONS_BASE_ID"), getenv("TEABLE_SUBMISSIONS_TABLE_ID"), NULL, &all_subs);
        // Simple top 5 sort (use qsort for prod)
        // Enrich with usernames (query users for each—inefficient but simple)
        cJSON *top = cJSON_CreateArray();
        for (int i = 0; i < cJSON_GetArraySize(all_subs) && i < 5; i++) {
            cJSON *sub = cJSON_Duplicate(cJSON_GetArrayItem(all_subs, i), 1);
            cJSON *fields = cJSON_GetObjectItem(sub, "fields");
            char ufilter[128]; snprintf(ufilter, sizeof(ufilter), "{\"id\": \"%s\"}", cJSON_GetObjectItem(fields, "user_id")->valuestring);
            cJSON *urec = NULL;
            teable_query(getenv("TEABLE_USERS_BASE_ID"), getenv("TEABLE_USERS_TABLE_ID"), ufilter, &urec);
            cJSON_AddStringToObject(fields, "username", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetArrayItem(urec, 0), "fields"), "username")->valuestring);
            cJSON_AddItemToArray(top, sub);
            cJSON_Delete(urec);
        }
        cJSON *resp = cJSON_CreateObject();
        cJSON_AddItemToObject(resp, "my_subs", my_subs);
        cJSON_AddItemToObject(resp, "top_scores", top);
        cJSON_Delete(all_subs);
        return (send_json_response(connection, MHD_HTTP_OK, resp) == MHD_YES) ? MHD_YES : MHD_NO;
    }

    cJSON_Delete(req_json);
    return (send_simple_status(connection, MHD_HTTP_NOT_FOUND) == MHD_YES) ? MHD_YES : MHD_NO;
}

int main(int argc, char *argv[]) {
    if (sodium_init() < 0) {
        fprintf(stderr, "Failed to initialize libsodium\n");
        return 1;
    }
    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL,
                                                  &handle_request, NULL, MHD_OPTION_END);
    if (!daemon) return 1;
    printf("Server running on port %d. Press Enter to quit.\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}
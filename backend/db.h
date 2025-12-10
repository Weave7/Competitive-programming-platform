// db.h
#ifndef DB_H
#define DB_H

#include <stdbool.h>
#include <cjson/cJSON.h>

// Initialize global DB connection pool / handle.
// Returns true on success.
bool db_init(void);

// User-related operations
// Returns a newly allocated cJSON object with fields { id, username, password_hash } or NULL.
cJSON *db_find_user_by_username(const char *username);

// Inserts a new user with username + password_hash.
// On success, returns newly allocated cJSON object { id, username, password_hash }.
// Returns NULL on failure.
cJSON *db_insert_user(const char *username, const char *password_hash);

// Submissions
// Insert submission; returns true on success.
bool db_insert_submission(int user_id, const char *problem_id, const char *code, int score, const char *status);

// Get submissions for a given user. Returns a cJSON array of records.
// Caller owns the returned cJSON*.
cJSON *db_get_submissions_by_user(int user_id);

// Get top N submissions sorted by score desc.
// Returns cJSON array of records joined with usernames: each item has
// { id, user_id, username, problem_id, score, status }.
cJSON *db_get_top_submissions(int limit);

#endif // DB_H

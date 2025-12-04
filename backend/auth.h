// auth.h
#ifndef AUTH_H
#define AUTH_H
#include <stdbool.h>
char* auth_login(const char *username, const char *password);  // Returns user_id or NULL
char* auth_register(const char *username, const char *password); // Returns user_id or NULL on failure
#endif
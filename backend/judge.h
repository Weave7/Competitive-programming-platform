// judge.h
#ifndef JUDGE_H
#define JUDGE_H
#include <stdbool.h>
typedef struct { double score; char status[4]; } JudgeResult;
JudgeResult judge_submission(const char *code, const char *problem_id);
#endif
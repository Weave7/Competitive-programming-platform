// judge.c
#include "judge.h"
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Read entire file into a malloc'd, NUL-terminated buffer. Caller must free.
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t r = fread(buf, 1, sz, f);
    buf[r] = '\0';
    fclose(f);
    return buf;
}

// Trim trailing whitespace/newlines in place
static void trim_trailing(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t')) {
        s[n-1] = '\0';
        n--;
    }
}

JudgeResult judge_submission(const char *code, const char *problem_id) {
    JudgeResult res = {0.0, "WA"};
    // Support four problems with IDs "1", "2", "3", "4"
    if (!(strcmp(problem_id, "1") == 0 || strcmp(problem_id, "2") == 0 || strcmp(problem_id, "3") == 0 || strcmp(problem_id, "4") == 0)) {
        return res;
    }

    // Read input and expected output from files under ./tests/<problem_id>.in/.out
    char inpath[256];
    char outpath[256];
    snprintf(inpath, sizeof(inpath), "tests/%s.in", problem_id);
    snprintf(outpath, sizeof(outpath), "tests/%s.out", problem_id);

    // read_file() is provided at file scope above

    char *input = read_file(inpath);
    char *expected = read_file(outpath);
    if (!input || !expected) {
        free(input); free(expected);
        return res; // missing test files -> WA
    }

    int in_fds[2];
    int out_fds[2];
    if (pipe(in_fds) == -1) { free(input); free(expected); return res; }
    if (pipe(out_fds) == -1) { close(in_fds[0]); close(in_fds[1]); free(input); free(expected); return res; }

    pid_t pid = fork();
    if (pid == -1) { close(in_fds[0]); close(in_fds[1]); close(out_fds[0]); close(out_fds[1]); free(input); free(expected); return res; }

    // Create a temporary directory for compilation and execution
    char tmpdir_template[] = "/tmp/judgeXXXXXX";
    char *tmpdir = mkdtemp(tmpdir_template);
    if (!tmpdir) { free(input); free(expected); return res; }

    char srcpath[512];
    char binpath[512];
    snprintf(srcpath, sizeof(srcpath), "%s/submission.cpp", tmpdir);
    snprintf(binpath, sizeof(binpath), "%s/prog", tmpdir);

    FILE *sf = fopen(srcpath, "wb");
    if (!sf) { free(input); free(expected); rmdir(tmpdir); return res; }
    fwrite(code, 1, strlen(code), sf);
    fclose(sf);

    // Compile the submitted C++ code
    pid_t cpid = fork();
    if (cpid == -1) { free(input); free(expected); unlink(srcpath); rmdir(tmpdir); return res; }
    if (cpid == 0) {
        execlp("g++", "g++", "-std=c++17", "-O2", "-o", binpath, srcpath, (char *)NULL);
        _exit(127);
    }
    int cstatus = 0;
    waitpid(cpid, &cstatus, 0);
    if (!(WIFEXITED(cstatus) && WEXITSTATUS(cstatus) == 0)) {
        // compilation error -> treat as WA
        unlink(srcpath);
        rmdir(tmpdir);
        free(input); free(expected);
        return res;
    }

    // Now run the compiled binary with resource limits
    pid_t runpid = fork();
    if (runpid == -1) { unlink(srcpath); unlink(binpath); rmdir(tmpdir); free(input); free(expected); return res; }
    if (runpid == 0) {
        struct rlimit cpu_lim = {1, 1};
        setrlimit(RLIMIT_CPU, &cpu_lim);
        struct rlimit mem_lim = {64 * 1024 * 1024, 64 * 1024 * 1024};
        setrlimit(RLIMIT_AS, &mem_lim);

        // stdin from in_fds[0]
        close(in_fds[1]);
        dup2(in_fds[0], STDIN_FILENO);
        close(in_fds[0]);

        // stdout to out_fds[1]
        close(out_fds[0]);
        dup2(out_fds[1], STDOUT_FILENO);
        close(out_fds[1]);

        execl(binpath, binpath, (char *)NULL);
        _exit(127);
    }

    // Parent
    close(in_fds[0]);
    close(out_fds[1]);

    // write input to child's stdin
    size_t written = 0; size_t inlen = strlen(input);
    while (written < inlen) {
        ssize_t w = write(in_fds[1], input + written, inlen - written);
        if (w <= 0) break;
        written += (size_t)w;
    }
    close(in_fds[1]);

    // read child's stdout into dynamic buffer
    char *outbuf = NULL; size_t outsize = 0;
    char tmp[1024];
    while (1) {
        ssize_t r = read(out_fds[0], tmp, sizeof(tmp));
        if (r <= 0) break;
        char *newb = realloc(outbuf, outsize + r + 1);
        if (!newb) break;
        outbuf = newb;
        memcpy(outbuf + outsize, tmp, r);
        outsize += r;
        outbuf[outsize] = '\0';
    }
    close(out_fds[0]);

    int status = 0;
    waitpid(runpid, &status, 0);

    // use trim_trailing() defined at file scope
    if (outbuf) trim_trailing(outbuf);
    trim_trailing(expected);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && outbuf && strcmp(outbuf, expected) == 0) {
        res.score = 100.0;
        strcpy(res.status, "AC");
    } else {
        strcpy(res.status, "WA");
    }

    unlink(srcpath);
    unlink(binpath);
    rmdir(tmpdir);
    free(input); free(expected); free(outbuf);
    return res;
}
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>

#define BUFFER_SIZE (256 * 1024)

static int test_memory(void)
{
    uint8_t *buffer = malloc(BUFFER_SIZE);

    if (buffer == NULL)
        return -1;

    for (size_t i = 0; i < BUFFER_SIZE; i++)
        buffer[i] = (uint8_t)(i & 0xFF);

    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i] != (uint8_t)(i & 0xFF)) {
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 0;
}

static int test_file(void)
{
    const char *path = "/tmp/p4linux-test.txt";
    const char *message = "P4Linux native application";
    char buffer[128];

    FILE *fp = fopen(path, "w");

    if (fp == NULL)
        return -1;

    int written = fputs(message, fp);
    int closed = fclose(fp);

    if (written == EOF || closed == EOF)
        return -1;

    fp = fopen(path, "r");

    if (fp == NULL)
        return -1;

    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fclose(fp);
        return -1;
    }

    fclose(fp);
    unlink(path);

    return strcmp(buffer, message) == 0 ? 0 : -1;
}

static int test_timer(void)
{
    struct timespec start, end;
    struct timespec delay = {1, 0};

    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
        return -1;

    if (nanosleep(&delay, NULL) != 0)
        return -1;

    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0)
        return -1;

    double elapsed =
        (double)(end.tv_sec - start.tv_sec) +
        (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Elapsed time: %.3f seconds\n", elapsed);

    return elapsed >= 1.0 ? 0 : -1;
}

int main(void)
{
    struct utsname info;
    int failures = 0;

    printf("\n");
    printf("====================================\n");
    printf(" P4Linux Native Application v0.1.0\n");
    printf("====================================\n\n");

    if (uname(&info) == 0) {
        printf("System       : %s\n", info.sysname);
        printf("Kernel       : %s\n", info.release);
        printf("Architecture : %s\n", info.machine);
    } else {
        failures++;
    }

    printf("Process ID   : %ld\n", (long)getpid());
    printf("Parent PID   : %ld\n", (long)getppid());
    printf("Pointer size : %zu bits\n",
           sizeof(void *) * 8);

    printf("\nRunning tests...\n\n");

    if (test_memory() == 0) {
        printf("[PASS] Dynamic memory (256 KiB)\n");
    } else {
        printf("[FAIL] Dynamic memory\n");
        failures++;
    }

    if (test_file() == 0) {
        printf("[PASS] File operations\n");
    } else {
        printf("[FAIL] File operations\n");
        failures++;
    }

    if (test_timer() == 0) {
        printf("[PASS] Timer and sleep\n");
    } else {
        printf("[FAIL] Timer and sleep\n");
        failures++;
    }

    printf("\n");

    if (failures == 0)
        printf("[SUCCESS] All tests passed\n");
    else
        printf("[ERROR] %d tests failed\n", failures);

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


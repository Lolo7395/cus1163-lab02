#include "proc_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

int is_number(const char* str) {
    if (str == NULL || *str == '\0') return 0;
    for (const unsigned char *p = (const unsigned char*)str; *p; ++p) {
        if (!isdigit(*p)) return 0;
    }
    return 1;
}

int list_process_directories(void) {
    DIR *dir = opendir("/proc");
    if (!dir) {
        fprintf(stderr, "ERROR: opendir(/proc) failed: %s\n", strerror(errno));

        return -1;
    }

    struct dirent *entry;
    size_t count = 0;

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    while ((entry = readdir(dir)) != NULL) {
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }

    if (closedir(dir) != 0) {
        fprintf(stderr, "ERROR: closedir(/proc) failed: %s\n", strerror(errno));
        return -1;
    }

    printf("Found %zu process directories\n", count);
    printf("SUCCESS: Process directories listed!\n");
    return 0;
}

int read_file_with_syscalls(const char* filename) {
    int fd;
    char buffer[4096];
    ssize_t bytes_read;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "ERROR: open(%s) failed: %s\n", filename, strerror(errno));
        return -1;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    if (bytes_read < 0) {
        fprintf(stderr, "ERROR: read(%s) failed: %s\n", filename, strerror(errno));
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "ERROR: close(%s) failed: %s\n", filename, strerror(errno));
        return -1;
    }

    return 0;
}

int read_file_with_library(const char* filename) {
    FILE *fp;
    char buffer[4096];

    fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: fopen(%s) failed: %s\n", filename, strerror(errno));
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        fputs(buffer, stdout);
    }

    if (fclose(fp) != 0) {
        fprintf(stderr, "ERROR: fclose(%s) failed: %s\n", filename, strerror(errno));
        return -1;
    }

    return 0;
}

int read_process_info(const char* pid) {
    char filepath[256];

    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);
    printf("\n--- Process Information for PID %s ---\n", pid);

    if (read_file_with_syscalls(filepath) != 0) {
        fprintf(stderr, "WARNING: Could not read %s\n", filepath);
    }

    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);
    printf("\n--- Command Line ---\n");

    if (read_file_with_syscalls(filepath) != 0) {
        fprintf(stderr, "WARNING: Could not read %s\n", filepath);
        printf("\n");
        return -1;
    }

    printf("\n");
    printf("SUCCESS: Process information read!\n");
    return 0;
}

int show_system_info(void) {
    int line_count = 0;
    const int MAX_LINES = 10;

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);

    FILE *cpu = fopen("/proc/cpuinfo", "r");
    if (!cpu) {
        fprintf(stderr, "ERROR: fopen(/proc/cpuinfo) failed: %s\n", strerror(errno));
    } else {
        char line[4096];
        line_count = 0;
        while (line_count < MAX_LINES && fgets(line, sizeof(line), cpu)) {
            fputs(line, stdout);
            line_count++;
        }
        if (fclose(cpu) != 0) {
            fprintf(stderr, "ERROR: fclose(/proc/cpuinfo) failed: %s\n", strerror(errno));
        }
    }

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);

    FILE *mem = fopen("/proc/meminfo", "r");
    if (!mem) {
        fprintf(stderr, "ERROR: fopen(/proc/meminfo) failed: %s\n", strerror(errno));
    } else {
        char line[4096];
        line_count = 0;
        while (line_count < MAX_LINES && fgets(line, sizeof(line), mem)) {
            fputs(line, stdout);
            line_count++;
        }
        if (fclose(mem) != 0) {
            fprintf(stderr, "ERROR: fclose(/proc/meminfo) failed: %s\n", strerror(errno));
        }
    }

    printf("SUCCESS: System information displayed!\n");
    return 0;
}

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

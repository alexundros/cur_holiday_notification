#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#define MAX_PATH 1024
#endif

#include "utils/ini/ini_utils.h"
#include "utils/xml/xml_utils.h"

#ifdef _WIN32

static inline char* get_app_dir() {
    static char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, path, sizeof(path));
    if (len == 0 || len >= sizeof(path)) return NULL;
    for (int i = len - 1; i >= 0; --i) {
        if (path[i] == '\\') {
            path[i] = '\0';
            break;
        }
    }
    return path;
}

static inline int get_full_path(const char* relative, char* full, size_t size) {
    if (GetFullPathName(relative, size, full, NULL) == 0) {
        return -1;
    }
    return 0;
}

#else

static inline char* get_app_dir() {
    static char path[MAX_PATH];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) return NULL;
    path[len] = '\0';
    char* slash = strrchr(path, '/');
    if (slash) *slash = '\0';
    return path;
}

static inline int get_full_path(const char* relative, char* full, size_t size) {
    if (realpath(relative, full) == NULL) {
        return -1;
    }
    return 0;
}

#endif

static inline int file_exists(const char* path) {
    struct stat buf;
    return (stat(path, &buf) == 0);
}

static inline struct timespec get_timespec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

void print_elapsed_time(char* buf, char* text, const struct timespec* start, const struct timespec* end) {
    long dts = (end->tv_sec - start->tv_sec) * 1e9 + (end->tv_nsec - start->tv_nsec);
    snprintf(buf, 1024, "# %s: %.9f сек.\n", text, dts / 1e9);
    fputs(buf, stdout);
}

char* get_xml_file(int argc, char* argv[]) {
    char path[MAX_PATH];
    if (argc > 1) {
        if (!file_exists(argv[1])) {
            fprintf(stderr, "# %s не найден\n", argv[1]);
            return NULL;
        }
        get_full_path(argv[1], path, sizeof(path));
        return strdup(path);
    } else {
        char buf[MAX_PATH];
        printf("# XML файл: ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            return NULL;
        }
        buf[strcspn(buf, "\r\n")] = 0;
        if (strlen(buf) == 0) {
            return get_xml_file(argc, argv);
        }
        if (!file_exists(buf)) {
            fprintf(stderr, "# %s не найден\n", buf);
            return get_xml_file(argc, argv);
        }
        get_full_path(buf, path, sizeof(path));
        return strdup(path);
    }
}

int get_config(config_t* config, char* text_buf, const char* appdir, const char* workdir) {
    const char* name = "cur_holiday_notification.cfg";
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%s%c%s", workdir, PATH_SEP, name);
    if (!file_exists(path)) {
        snprintf(path, MAX_PATH, "%s%c%s", appdir, PATH_SEP, name);
    }
    if (!file_exists(path)) {
        fprintf(stderr, "# Конфиг не найден\n");
        return 1;
    }

    snprintf(text_buf, 1024, "# Используем конфиг: %s\n", path);
    fputs(text_buf, stdout);

    if (!init_config(config, path)) {
        fprintf(stderr, "# Не удалось загрузить конфиг %s\n", path);
        return 2;
    }
    return 0;
}

int write_results(char* text_buf, const char* workdir, const result_list_t* results) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%s%cc_project.out", workdir, PATH_SEP);
    FILE* fo = fopen(path, "w");
    if (!fo) {
        fprintf(stderr, "# Не могу создать/записать файл результата: %s\n", path);
        return 1;
    }

    if (results->count) {
        fputs("# Результат:\n", stdout);
        for (int i = 0; i < results->count; i++) {
            char* code = results->items[i].code;
            char* date = results->items[i].date;
            snprintf(text_buf, 1024, "%s = %s\n", code, date);
            fputs(text_buf, fo);
            fputs(text_buf, stdout);
        }
    } else {
        fputs("# Нет результата\n", stdout);
    }
    fclose(fo);

    snprintf(text_buf, 1024, "# Результат сохранен в файл: %s\n", path);
    fputs(text_buf, stdout);
    return 0;
}

int main_process(int argc, char* argv[], char* text_buf, const char* appdir, const char* workdir) {
    char* xml_path = get_xml_file(argc, argv);
    if (!xml_path) {
        return 1;
    }
    struct timespec start = get_timespec();

    config_t config;
    if (get_config(&config, text_buf, appdir, workdir)) {
        return 2;
    }
    struct timespec end_get_config = get_timespec();

    result_list_t results;
    if (process_xml(text_buf, xml_path, &config, &results)) {
        fprintf(stderr, "# Ошибка при обработке XML %s\n", xml_path);
        return 3;
    }
    struct timespec end_process_xml = get_timespec();

    if (write_results(text_buf, workdir, &results)) {
        return 4;
    }
    struct timespec end = get_timespec();

    print_elapsed_time(text_buf, "* get config", &start, &end_get_config);
    print_elapsed_time(text_buf, "* process xml", &end_get_config, &end_process_xml);
    print_elapsed_time(text_buf, "* process results", &end_process_xml, &end);
    print_elapsed_time(text_buf, "Обработка завершена", &start, &end);
    return 0;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    if (!SetConsoleOutputCP(65001)) {
        printf("Failed to set console code page: %lu\n", GetLastError());
        return 1;
    }
#endif
    setvbuf(stdout, NULL, _IOFBF, 8192);

    char text_buf[1024];
    char* appdir = get_app_dir();
    if (!appdir) return 1;

    char workdir_buf[1024];
    if (!getcwd(workdir_buf, sizeof(workdir_buf))) return 2;
    char* workdir = strdup(workdir_buf);

    snprintf(text_buf, 1024, "# Директория приложения: %s\n", appdir);
    fputs(text_buf, stdout);
    snprintf(text_buf, 1024, "# Рабочая директория: %s\n", workdir);
    fputs(text_buf, stdout);

    if (main_process(argc, argv, text_buf, appdir, workdir)) {
        fprintf(stderr, "# Программа прервана по ошибке\n");
        return 3;
    }

    fflush(stdout);

    if (argc <= 2 || strcmp(argv[2], "true") != 0) {
        fputs("# Нажмите <Enter> для выхода", stdout);
        getchar();
    }
    return 0;
}

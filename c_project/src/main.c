#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "utils/ini_utils.h"
#include "utils/xml_utils.h"

static inline struct timespec get_timespec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

static inline void diff_print(char* text_buf, char* text, const struct timespec* a, const struct timespec* b) {
    long diff = (b->tv_sec - a->tv_sec) * 1000000000L + (b->tv_nsec - a->tv_nsec);
    snprintf(text_buf, 1024, "# %s: %.9f сек.", text, diff / 1e9);
    puts(text_buf);
}

static int file_exists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

static char* get_xml_file(int argc, char* argv[]) {
    if (argc > 1) {
        if (!file_exists(argv[1])) {
            fprintf(stderr, "# %s не найден\n", argv[1]);
            return NULL;
        }
        return strdup(argv[1]);
    } else {
        char buf[512];
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
        return strdup(buf);
    }
}

static char* get_config_path(const char* appdir) {
    const char* name = "cur_holiday_notification.cfg";
    char buf[1024];
    snprintf(buf, 1024, "%s", name);
    if (file_exists(buf)) {
        return strdup(buf);
    }
    snprintf(buf, 1024, "%s/%s", appdir, name);
    if (file_exists(buf)) {
        return strdup(buf);
    }
    return NULL;
}

#ifdef _WIN32

char* get_appdir() {
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

#else

#include <libgen.h>
#include <unistd.h>

char* get_appdir() {
    static char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len == -1) return NULL;
    path[len] = '\0';
    char* slash = strrchr(path, '/');
    if (slash) *slash = '\0';
    return path;
}

#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    if (!SetConsoleOutputCP(65001)) {
        printf("Failed to set console code page: %lu\n", GetLastError());
        return 1;
    }
#endif

    struct timespec ts_start = get_timespec();

    char text_buf[1024];

    char* appdir = get_appdir();
    if (!appdir) return 1;
    char workdir_buf[1024];
    if (!getcwd(workdir_buf, sizeof(workdir_buf))) return 1;
    char* workdir = strdup(workdir_buf);

    snprintf(text_buf, 1024, "# Директория приложения: %s", appdir);
    puts(text_buf);
    snprintf(text_buf, 1024, "# Рабочая директория: %s", workdir);
    puts(text_buf);

    // Конфигурация

    struct timespec ts_config_s = get_timespec();

    char* config_path = get_config_path(appdir);
    if (!config_path) {
        fprintf(stderr, "# Конфиг не найден\n");
        return 2;
    }
    snprintf(text_buf, 1024, "# Используем конфиг: %s", config_path);
    puts(text_buf);

    config_t config;
    if (!init_config(&config, config_path)) {
        fprintf(stderr, "# Не удалось загрузить конфиг %s\n", config_path);
        return 3;
    }

    struct timespec ts_config_e = get_timespec();

    // Обработка XML

    char* xml_path = get_xml_file(argc, argv);
    if (!xml_path) {
        fprintf(stderr, "# Ошибка: XML файл не указан\n");
        return 4;
    }
    snprintf(text_buf, 1024, "# Обработка: %s", xml_path);
    puts(text_buf);

    result_list_t results;
    init_result_list(&results);
    if (process_xml(xml_path, &config, &results) > 0) {
        fprintf(stderr, "# Ошибка при обработке XML %s\n", xml_path);
        return 5;
    }

    struct timespec ts_xml_e = get_timespec();

    // Вывод результата

    char out_path[1024];
    snprintf(out_path, sizeof(out_path), "%s/c_project.out", workdir);
    FILE* fo = fopen(out_path, "w");
    if (!fo) {
        fprintf(stderr, "# Не могу создать/записать файл результата: %s\n", out_path);
        return 6;
    }
    if (results.count > 0) {
        puts("# Результат:");
        for (int i = 0; i < results.count; i++) {
            snprintf(text_buf, 1024, "%s = %s", results.items[i].code, results.items[i].date);
            fputs(text_buf, fo);
            fputc('\n', fo);
            puts(text_buf);
        }
    } else {
        printf("# Нет результата\n");
    }
    fclose(fo);

    snprintf(text_buf, 1024, "# Результат сохранен в файл: %s", out_path);
    puts(text_buf);

    struct timespec ts_out_e = get_timespec();

    // Завершение

    struct timespec ts_end = get_timespec();

    free_result_list(&results);
    free(config_path);
    free(xml_path);

    diff_print(text_buf, "* config", &ts_config_s, &ts_config_e);
    diff_print(text_buf, "* process xml", &ts_config_e, &ts_xml_e);
    diff_print(text_buf, "* process results", &ts_xml_e, &ts_out_e);
    diff_print(text_buf, "Обработка завершена", &ts_start, &ts_end);

    if (argc <= 2 || strcmp(argv[2], "true") != 0) {
        puts("# Нажмите <Enter> для выхода");
        getchar();
    }

    return 0;
}

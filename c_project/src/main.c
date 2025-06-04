#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>

#include "utils/ini_utils.h"
#include "utils/xml_utils.h"

static inline long diff_nsec(const struct timespec *a, const struct timespec *b)
{
    return (b->tv_sec - a->tv_sec) * 1000000000L + (b->tv_nsec - a->tv_nsec);
}

static inline struct timespec get_timespec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

static inline void diff_nsec_print(char *text, const struct timespec *a, const struct timespec *b)
{
    printf("# %s: %.9f сек.\n", text, diff_nsec(a, b) / 1e9);
}

static int file_exists(const char *path)
{
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

static char *get_xml_file(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!file_exists(argv[1]))
        {
            fprintf(stderr, "# %s не найден\n", argv[1]);
            return NULL;
        }
        return strdup(argv[1]);
    }
    else
    {
        char buf[512];
        printf("# XML файл: ");
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            return NULL;
        }
        buf[strcspn(buf, "\r\n")] = 0;
        if (strlen(buf) == 0)
        {
            return get_xml_file(argc, argv);
        }
        if (!file_exists(buf))
        {
            fprintf(stderr, "# %s не найден\n", buf);
            return get_xml_file(argc, argv);
        }
        return strdup(buf);
    }
}

static char *get_config_path(const char *appdir)
{
    const char *name = "cur_holiday_notification.cfg";
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", name);
    if (file_exists(buf))
    {
        return strdup(buf);
    }
    snprintf(buf, sizeof(buf), "%s/%s", appdir, name);
    if (file_exists(buf))
    {
        return strdup(buf);
    }
    return NULL;
}

static char *get_appdir(const char *argv0)
{
    char buf[1024];
    strncpy(buf, argv0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *p = strrchr(buf, '/');
    if (!p)
    {
        p = strrchr(buf, '\\');
    }
    if (p)
    {
        *p = '\0';
        return strdup(buf);
    }
    else
    {
        return strdup(".");
    }
}

int main(int argc, char *argv[])
{
    setlocale(LC_CTYPE, ".UTF8");
    struct timespec ts_start = get_timespec();

    char *appdir = get_appdir(argv[0]);
    char workdir_buf[1024];
    if (!getcwd(workdir_buf, sizeof(workdir_buf)))
    {
        return 1;
    }
    char *workdir = strdup(workdir_buf);

    // printf("# Директория приложения: %s\n", appdir);
    // printf("# Рабочая директория: %s\n", workdir);

    // Конфигурация

    struct timespec ts_config_s1 = get_timespec();
    char *config_path = get_config_path(appdir);
    if (!config_path)
    {
        fprintf(stderr, "# Конфиг не найден\n");
        return 2;
    }
    // printf("# Используем конфиг: %s\n", config_path);

    config_t config;
    struct timespec ts_config_s2 = get_timespec();
    if (!init_config(&config, config_path))
    {
        fprintf(stderr, "# Не удалось загрузить конфиг %s\n", config_path);
        return 3;
    }
    struct timespec ts_config_e = get_timespec();

    // Обработка XML

    // char *xml_path = get_xml_file(argc, argv);
    // if (!xml_path)
    // {
    //     fprintf(stderr, "# Ошибка: XML файл не указан\n");
    //     return 4;
    // }
    // // printf("# Обработка: %s\n", xml_path);

    // ResultList results;
    // init_result_list(&results);
    // if (process_xml(xml_path, &config, &results) < 0)
    // {
    //     fprintf(stderr, "# Ошибка при обработке XML %s\n", xml_path);
    //     return 5;
    // }
    // struct timespec ts_xml_e = get_timespec();

    // Вывод результата

    // char out_path[1024];
    // snprintf(out_path, sizeof(out_path), "%s/c_project.out", workdir);
    // FILE *fo = fopen(out_path, "w");
    // if (!fo)
    // {
    //     fprintf(stderr, "# Не могу создать/записать файл результата: %s\n", out_path);
    //     return 6;
    // }

    // if (results.count > 0)
    // {
    //     // printf("# Результат:\n");
    //     for (int i = 0; i < results.count; i++)
    //     {
    //         char *code = results.items[i].code;
    //         char *date = results.items[i].date;
    //         fprintf(fo, "%s = %s\n", code, date);
    //         // printf("%s = %s\n", code, date);
    //     }
    // }
    // else
    // {
    //     printf("# Нет результата\n");
    // }

    // // printf("# Результат сохранен в файл: %s\n", out_path);
    // fclose(fo);
    // struct timespec ts_out_e = get_timespec();

    // Завершение

    // free_result_list(&results);
    //free(appdir);
    //free(workdir);
    //free(config_path);
    //free(xml_path);

    struct timespec ts_end = get_timespec();
    diff_nsec_print("* config (1)", &ts_config_s1, &ts_config_e);
    diff_nsec_print("* config (2)", &ts_config_s2, &ts_config_e);
    // diff_nsec_print("* xml", &ts_config_e, &ts_xml_e);
    diff_nsec_print("Обработка завершена", &ts_start, &ts_end);

    if (argc <= 2 || strcmp(argv[2], "true") != 0)
    {
        printf("# Нажмите <Enter> для выхода\n");
        getchar();
    }

    return 0;
}

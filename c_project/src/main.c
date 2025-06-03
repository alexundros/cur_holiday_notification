#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

// #include "ini_utils.h"
// #include "xml_utils.h"
#include <locale.h>

// // Проверка существования файла
// static int file_exists(const char *path)
// {
//     struct stat buffer;
//     return (stat(path, &buffer) == 0);
// }

// // Запрос пути к XML-файлу: либо аргументом из argv, либо ввод пользователя
// static char *get_xml_file(int argc, char *argv[])
// {
//     if (argc > 1)
//     {
//         if (!file_exists(argv[1]))
//         {
//             fprintf(stderr, "# %s не найден\n", argv[1]);
//             return NULL;
//         }
//         return strdup(argv[1]);
//     }
//     else
//     {
//         char buf[512];
//         printf("# XML файл: ");
//         if (fgets(buf, sizeof(buf), stdin) == NULL)
//         {
//             return NULL;
//         }
//         buf[strcspn(buf, "\r\n")] = 0;
//         if (strlen(buf) == 0)
//         {
//             return get_xml_file(argc, argv);
//         }
//         if (!file_exists(buf))
//         {
//             fprintf(stderr, "# %s не найден\n", buf);
//             return get_xml_file(argc, argv);
//         }
//         return strdup(buf);
//     }
// }

// // Получаем путь к файлу cur_holiday_notification.cfg
// static char *get_config_path(const char *appdir)
// {
//     const char *cfg_name = "cur_holiday_notification.cfg";
//     char candidate[1024];

//     // Сначала ищем в рабочей папке
//     snprintf(candidate, sizeof(candidate), "%s", cfg_name);
//     if (file_exists(candidate))
//     {
//         return strdup(candidate);
//     }
//     // Иначе в папке приложения
//     snprintf(candidate, sizeof(candidate), "%s/%s", appdir, cfg_name);
//     if (file_exists(candidate))
//     {
//         return strdup(candidate);
//     }
//     return NULL;
// }

// Получаем директорию приложения (путь до исполняемого файла)
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

static inline long diff_nsec(const struct timespec *a, const struct timespec *b)
{
    return (b->tv_sec - a->tv_sec) * 1000000000L + (b->tv_nsec - a->tv_nsec);
}

int main(int argc, char *argv[])
{
    setlocale(LC_CTYPE, ".UTF8");

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    char *appdir = get_appdir(argv[0]);
    char workdir_buf[512];
    if (!getcwd(workdir_buf, sizeof(workdir_buf)))
    {
        fprintf(stderr, "# Не удалось определить текущую директорию\n");
        return 1;
    }
    char *workdir = strdup(workdir_buf);

    printf("# Директория приложения: %s\n", appdir);
    printf("# Рабочая директория: %s\n", workdir);

    // // Файл конфигурации
    // char* cfg_path = get_config_path(appdir);
    // if (!cfg_path) {
    //     fprintf(stderr, "# Конфиг cur_holiday_notification.cfg не найден\n");
    //     free(appdir);
    //     free(workdir);
    //     free(xml_path);
    //     return 1;
    // }
    // printf("# Используем конфиг: %s\n", cfg_path);

    // // Загружаем конфигурацию
    // FeaturesConfig fcfg;
    // ItemsConfig icfg;
    // if (load_config(cfg_path, &fcfg, &icfg) < 0) {
    //     fprintf(stderr, "# Не удалось загрузить конфиг %s\n", cfg_path);
    //     free(appdir);
    //     free(workdir);
    //     free(xml_path);
    //     free(cfg_path);
    //     return 1;
    // }

    // // XML-файл
    // char* xml_path = get_xml_file(argc, argv);
    // if (!xml_path) {
    //     fprintf(stderr, "# Ошибка: XML файл не указан\n");
    //     free(appdir);
    //     free(workdir);
    //     return 1;
    // }
    // printf("# Обработка: %s\n", xml_path);

    // // Обрабатываем XML и собираем результаты
    // ResultList results;
    // init_result_list(&results);
    // if (process_xml(xml_path, &fcfg, &icfg, &results) < 0) {
    //     fprintf(stderr, "# Ошибка при обработке XML %s\n", xml_path);
    //     free_result_list(&results);
    //     free(appdir);
    //     free(workdir);
    //     free(xml_path);
    //     free(cfg_path);
    //     return 1;
    // }

    // // Путь к выходному файлу (c_project.out в рабочей директории)
    // char out_path[1024];
    // snprintf(out_path, sizeof(out_path), "%s/c_project.out", workdir);
    // FILE* fo = fopen(out_path, "w");
    // if (!fo) {
    //     fprintf(stderr, "# Не могу создать/записать файл результата: %s\n", out_path);
    //     free_result_list(&results);
    //     free(appdir);
    //     free(workdir);
    //     free(xml_path);
    //     free(cfg_path);
    //     return 1;
    // }

    // if (results.count > 0) {
    //     printf("# Результат:\n");
    //     for (int i = 0; i < results.count; i++) {
    //         fprintf(fo, "%s = %s\n", results.items[i].code, results.items[i].date);
    //         printf("%s = %s\n", results.items[i].code, results.items[i].date);
    //     }
    // } else {
    //     printf("# Нет результата\n");
    // }
    // printf("# Результат сохранен в файл: %s\n", out_path);
    // fclose(fo);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long times_ns = diff_nsec(&t0, &t1);
    printf("# Обработка завершена: %.6f сек.\n", times_ns / 1e9);

    // // Освобождаем память
    // free_result_list(&results);
    // free(appdir);
    // free(workdir);
    // free(xml_path);
    // free(cfg_path);

    // // Ожидание нажатия Enter, если нет второго аргумента "true"
    // if (argc <= 2 || strcmp(argv[2], "true") != 0) {
    //     printf("# Нажмите <Enter> для выхода\n");
    //     getchar();
    // }

    return 0;
}

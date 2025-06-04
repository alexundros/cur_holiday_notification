#ifndef INI_UTILS_H
#define INI_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_ITEMS 256
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256

typedef struct
{
    bool HDay;
} FeaturesConfig;

typedef struct
{
    char key[MAX_KEY_LEN];
    char val[MAX_VAL_LEN];
} ItemPair;

typedef struct
{
    ItemPair items[MAX_ITEMS];
    int count;
} ItemsConfig;

// Callback для inih
int config_handler(void *user, const char *section, const char *name, const char *value);

// Функция загрузки конфигурации из INI-файла
int load_config(const char *cfg_path, FeaturesConfig *fcfg, ItemsConfig *icfg);

#endif // INI_UTILS_H

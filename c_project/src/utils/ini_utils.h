#ifndef INI_UTILS_H
#define INI_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
} config_item_t;

typedef struct {
    config_item_t* list;
    int count;
    int capacity;
} config_list_t;

typedef struct {
    bool HDay;
} config_features_t;

typedef struct {
    config_features_t features;
    config_list_t items;
} config_t;

bool init_config(config_t* config, const char* cfg_path);

#endif  // INI_UTILS_H

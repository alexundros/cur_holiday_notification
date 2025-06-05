#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ini/ini.h"
#include "ini_utils.h"

void config_items_init(config_list_t *items) {
    items->count = 0;
    items->capacity = 4;
    items->list = malloc(sizeof(config_item_t) * items->capacity);
}

void config_items_add(config_list_t *items, const char *key, const char *value) {
    if (items->count >= items->capacity) {
        items->capacity *= 2;
        items->list = realloc(items->list, sizeof(config_item_t) * items->capacity);
    }
    strncpy(items->list[items->count].key, key, MAX_KEY_LEN - 1);
    strncpy(items->list[items->count].value, value, MAX_VAL_LEN - 1);
    items->list[items->count].key[MAX_KEY_LEN - 1] = '\0';
    items->list[items->count].value[MAX_VAL_LEN - 1] = '\0';
    items->count++;
}

void config_items_free(config_list_t *items) {
    items->count = 0;
    items->capacity = 0;
    free(items->list);
    items->list = NULL;
}

int config_handler(void *user, const char *section, const char *name, const char *val) {
    config_t *config = (config_t *) user;
    if (strcmp(section, "features") == 0) {
        if (strcmp(name, "HDay") == 0) {
            config->features.HDay = (strcmp(val, "true") == 0);
        }
    } else if (strcmp(section, "items") == 0) {
        config_items_add(&config->items, name, val);
    }
    return 1;
}

bool init_config(config_t *config, const char *config_path) {
    memset(config, 0, sizeof(config_t));
    config_items_init(&config->items);
    if (ini_parse(config_path, config_handler, config) < 0) {
        config_items_free(&config->items);
        return false;
    }
    return true;
}

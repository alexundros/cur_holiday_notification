#include "ini_utils.h"
#include "ini/ini.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Временные структуры для передачи в callback
static FeaturesConfig tmp_feats;
static ItemsConfig   tmp_items;

// config_handler вызывается inih для каждой записи name=value
int config_handler(void* user, const char* section, const char* name, const char* value) {
    (void)user;

    if (strcmp(section, "features") == 0) {
        if (strcmp(name, "HDay") == 0) {
            if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
                tmp_feats.HDay = true;
            } else {
                tmp_feats.HDay = false;
            }
        }
    }
    else if (strcmp(section, "items") == 0) {
        if (tmp_items.count < MAX_ITEMS) {
            strncpy(tmp_items.items[tmp_items.count].key, name, MAX_KEY_LEN - 1);
            tmp_items.items[tmp_items.count].key[MAX_KEY_LEN - 1] = '\0';
            strncpy(tmp_items.items[tmp_items.count].val, value, MAX_VAL_LEN - 1);
            tmp_items.items[tmp_items.count].val[MAX_VAL_LEN - 1] = '\0';
            tmp_items.count++;
        }
    }
    return 1;
}

int load_config(const char* cfg_path, FeaturesConfig* fcfg, ItemsConfig* icfg) {
    tmp_feats.HDay = false;
    tmp_items.count = 0;

    if (ini_parse(cfg_path, config_handler, NULL) < 0) {
        return -1;
    }

    *fcfg = tmp_feats;
    *icfg = tmp_items;
    return 0;
}

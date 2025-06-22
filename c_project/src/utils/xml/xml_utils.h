#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <stdbool.h>

#include "../ini/ini_utils.h"

typedef struct {
    char code[128];
    char date[32];
} result_item_t;

typedef struct {
    result_item_t* items;
    int count;
    int capacity;
} result_list_t;

void init_result_list(result_list_t* rl);

void append_result(result_list_t* rl, const char* code, const char* date);

void free_result_list(result_list_t* rl);

int process_xml(char* text_buf, const char* xml_path, const config_t* config, result_list_t* rl);

#endif  // XML_UTILS_H

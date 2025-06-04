#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <stdbool.h>
#include "ini_utils.h"

typedef struct
{
    char code[128];
    char date[32];
} ResultItem;

typedef struct
{
    ResultItem *items;
    int count;
    int capacity;
} ResultList;

void init_result_list(ResultList *rl);
void append_result(ResultList *rl, const char *code, const char *date);
void free_result_list(ResultList *rl);

int process_xml(const char *xml_path, const config_t *config, ResultList *rl);

#endif // XML_UTILS_H

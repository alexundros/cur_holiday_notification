#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "ini/ini.h"
#include <locale.h>

#define MAX_ITEMS 100
#define MAX_NAME 128
#define CONFIG_FILE "cur_holiday_notification.cfg"
#define OUTPUT_FILE "c_project.out"

typedef struct
{
    char section[MAX_NAME];
    char key[MAX_NAME];
    char value[MAX_NAME];
} ConfigEntry;

typedef struct
{
    ConfigEntry items[MAX_ITEMS];
    int count;
} Config;

static int config_handler(void *user, const char *section, const char *name, const char *value)
{
    Config *pconfig = (Config *)user;
    if (pconfig->count >= MAX_ITEMS)
        return 0;

    strncpy(pconfig->items[pconfig->count].section, section, MAX_NAME - 1);
    strncpy(pconfig->items[pconfig->count].key, name, MAX_NAME - 1);
    strncpy(pconfig->items[pconfig->count].value, value, MAX_NAME - 1);
    pconfig->count++;
    return 1;
}

const char *get_config_value(Config *config, const char *section, const char *key)
{
    for (int i = 0; i < config->count; ++i)
    {
        if (strcmp(config->items[i].section, section) == 0 &&
            strcmp(config->items[i].key, key) == 0)
        {
            return config->items[i].value;
        }
    }
    return NULL;
}

// void free_config(Config *config) {
//     for (int i = 0; i < config->count; i++) {
//         free(config->keys[i]);
//         free(config->values[i]);
//     }
//     free(config->keys);
//     free(config->values);
// }

// int process_xml(const char *xml_file, Config *config, const char *output_path) {
//     xmlDoc *doc = xmlReadFile(xml_file, NULL, 0);
//     if (!doc) {
//         fprintf(stderr, "# Не удалось открыть XML файл: %s\n", xml_file);
//         return 1;
//     }

//     FILE *out = fopen(output_path, "w");
//     if (!out) {
//         fprintf(stderr, "# Не удалось создать файл результата\n");
//         xmlFreeDoc(doc);
//         return 1;
//     }

//     xmlNode *root = xmlDocGetRootElement(doc);
//     int result_found = 0;

//     for (xmlNode *cur = root->children; cur; cur = cur->next) {
//         if (cur->type == XML_ELEMENT_NODE && strcmp((char *)cur->name, "CUX50") == 0) {
//             xmlChar *reportDate = xmlGetProp(cur, (const xmlChar *)"ReportDate");

//             if (config->HDay) {
//                 for (xmlNode *group = cur->children; group; group = group->next) {
//                     if (group->type == XML_ELEMENT_NODE && strcmp((char *)group->name, "GROUP") == 0) {
//                         xmlChar *tg = xmlGetProp(group, (const xmlChar *)"TradeGroup");
//                         if (tg && strcmp((char *)tg, "H") == 0) {
//                             fprintf(out, "HDay = %s\n", reportDate);
//                             printf("HDay = %s\n", reportDate);
//                             xmlFree(tg);
//                             result_found = 1;
//                             break;
//                         }
//                         xmlFree(tg);
//                     }
//                 }
//             }

//             for (int i = 0; i < config->count; i++) {
//                 for (xmlNode *sec = cur->children; sec; sec = sec->next) {
//                     if (sec->type == XML_ELEMENT_NODE && strcmp((char *)sec->name, "SECURITY") == 0) {
//                         xmlChar *secName = xmlGetProp(sec, (const xmlChar *)"SecShortName");
//                         if (secName && strcmp((char *)secName, config->keys[i]) == 0) {
//                             fprintf(out, "%s = %s\n", config->values[i], reportDate);
//                             printf("%s = %s\n", config->values[i], reportDate);
//                             xmlFree(secName);
//                             result_found = 1;
//                             break;
//                         }
//                         xmlFree(secName);
//                     }
//                 }
//             }

//             xmlFree(reportDate);
//         }
//     }

//     fclose(out);
//     xmlFreeDoc(doc);

//     if (!result_found)
//         printf("# Нет результата\n");

//     printf("# Результат сохранен в файл: %s\n", output_path);
//     return 0;
// }

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, ".UTF8");

    if (argc < 2)
    {
        fprintf(stderr, "# Использование: %s <xml_file>\n", argv[0]);
        return 1;
    }

    const char *xml_file = argv[1];
    printf("# XML файл: %s\n", xml_file);

    Config config = {0};
    if (ini_parse(CONFIG_FILE, config_handler, &config) < 0)
    {
        fprintf(stderr, "# Не удалось загрузить конфиг: %s\n", CONFIG_FILE);
        return 1;
    }

    printf("# Используем конфиг: %s\n", CONFIG_FILE);

     const char* hday_val = get_config_value(config, "features", "HDay");
    int hday = (hday_val && strcmp(hday_val, "true") == 0);

    // int result = process_xml(xml_file, &config, OUTPUT_FILE);
    // free_config(&config);

    // return result;

    return 0;
}

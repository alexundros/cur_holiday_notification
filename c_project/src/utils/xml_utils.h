#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <stdbool.h>
#include "ini_utils.h"

// Структура одного результата: code (например, "CNY" или "HDay") и date ("YYYY-MM-DD")
typedef struct {
    char code[128];
    char date[32];
} ResultItem;

// Динамический список результатов
typedef struct {
    ResultItem* items;
    int count;
    int capacity;
} ResultList;

// Инициализация и работа со списком
void init_result_list(ResultList* rl);
void append_result(ResultList* rl, const char* code, const char* date);
void free_result_list(ResultList* rl);

/*
    process_xml:
      - xml_path: путь к XML-файлу
      - fcfg: информация о features (HDay)
      - icfg: информация по items (ключ→валюта)
      - rl: сюда добавляем найденные записи

    Возвращает 0 при успехе, <0 — при ошибке.
*/
int process_xml(const char* xml_path, const FeaturesConfig* fcfg, const ItemsConfig* icfg, ResultList* rl);

#endif // XML_UTILS_H

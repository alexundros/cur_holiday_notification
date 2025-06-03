#include "xml_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// подключаем libxml2
#include <libxml/parser.h>
#include <libxml/tree.h>

// Инициализация списка
void init_result_list(ResultList* rl) {
    rl->count = 0;
    rl->capacity = 16;
    rl->items = (ResultItem*)malloc(sizeof(ResultItem) * rl->capacity);
    if (rl->items == NULL) {
        fprintf(stderr, "# Ошибка выделения памяти для ResultList\n");
        exit(1);
    }
}

// Добавление элемента в список
void append_result(ResultList* rl, const char* code, const char* date) {
    if (rl->count >= rl->capacity) {
        rl->capacity *= 2;
        rl->items = (ResultItem*)realloc(rl->items, sizeof(ResultItem) * rl->capacity);
        if (rl->items == NULL) {
            fprintf(stderr, "# Ошибка перераспределения памяти для ResultList\n");
            exit(1);
        }
    }
    strncpy(rl->items[rl->count].code, code, sizeof(rl->items[rl->count].code) - 1);
    rl->items[rl->count].code[sizeof(rl->items[rl->count].code) - 1] = '\0';
    strncpy(rl->items[rl->count].date, date, sizeof(rl->items[rl->count].date) - 1);
    rl->items[rl->count].date[sizeof(rl->items[rl->count].date) - 1] = '\0';
    rl->count++;
}

// Освобождение памяти списка
void free_result_list(ResultList* rl) {
    if (rl->items) {
        free(rl->items);
        rl->items = NULL;
    }
    rl->count = 0;
    rl->capacity = 0;
}

/*
    Реализация process_xml с помощью libxml2.
*/
int process_xml(const char* xml_path, const FeaturesConfig* fcfg, const ItemsConfig* icfg, ResultList* rl) {
    // Инициализируем парсер libxml2 (можно вызвать один раз в main, но здесь для надёжности)
    xmlInitParser();

    // Загружаем XML-документ
    xmlDocPtr doc = xmlReadFile(xml_path, NULL, XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    if (doc == NULL) {
        return -1; // не удалось открыть или распарсить XML
    }

    // Берём корневой элемент <MICEX_DOC>
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == NULL || xmlStrcmp(root->name, (const xmlChar*)"MICEX_DOC") != 0) {
        xmlFreeDoc(doc);
        return -2; // корневой элемент не тот
    }

    // Проходим по всем дочерним элементам <CUX50> (именно прямые дети)
    for (xmlNodePtr cux = root->children; cux; cux = cux->next) {
        if (cux->type != XML_ELEMENT_NODE) continue;
        if (xmlStrcmp(cux->name, (const xmlChar*)"CUX50") != 0) continue;

        // Получаем атрибут ReportDate
        xmlChar* attrDate = xmlGetProp(cux, (const xmlChar*)"ReportDate");
        if (attrDate == NULL) {
            continue;
        }
        char reportDate[32];
        strncpy(reportDate, (const char*)attrDate, sizeof(reportDate)-1);
        reportDate[sizeof(reportDate)-1] = '\0';
        xmlFree(attrDate);

        bool foundInThisCux = false;

        // Если включён HDay, ищем <GROUP TradeGroup="H">
        if (fcfg->HDay) {
            for (xmlNodePtr grp = cux->children; grp; grp = grp->next) {
                if (grp->type != XML_ELEMENT_NODE) continue;
                if (xmlStrcmp(grp->name, (const xmlChar*)"GROUP") != 0) continue;

                xmlChar* tg = xmlGetProp(grp, (const xmlChar*)"TradeGroup");
                if (tg && xmlStrcmp(tg, (const xmlChar*)"H") == 0) {
                    append_result(rl, "HDay", reportDate);
                    xmlFree(tg);
                    foundInThisCux = true;
                    break;
                }
                if (tg) xmlFree(tg);
            }
            if (foundInThisCux) {
                continue; // переходим к следующему <CUX50>
            }
        }

        // Иначе — ищем по каждому ключу из icfg:
        for (int i = 0; i < icfg->count; i++) {
            const char* key = icfg->items[i].key; // это SecShortName
            const char* val = icfg->items[i].val; // это валюта

            // Для каждого <GROUP> внутри текущего <CUX50> пробегаем всех <SECURITY>
            bool itemMatched = false;
            for (xmlNodePtr grp = cux->children; grp; grp = grp->next) {
                if (grp->type != XML_ELEMENT_NODE) continue;
                if (xmlStrcmp(grp->name, (const xmlChar*)"GROUP") != 0) continue;

                for (xmlNodePtr sec = grp->children; sec; sec = sec->next) {
                    if (sec->type != XML_ELEMENT_NODE) continue;
                    if (xmlStrcmp(sec->name, (const xmlChar*)"SECURITY") != 0) continue;

                    xmlChar* ssn = xmlGetProp(sec, (const xmlChar*)"SecShortName");
                    if (ssn && strcmp((const char*)ssn, key) == 0) {
                        append_result(rl, val, reportDate);
                        xmlFree(ssn);
                        itemMatched = true;
                        break;
                    }
                    if (ssn) xmlFree(ssn);
                }
                if (itemMatched) break;
            }
            // Если нашли хотя бы один <SECURITY> с нужным SecShortName, не ищем остальные ключи
            if (itemMatched) {
                // Переходим к следующему ключу icfg
                continue;
            }
        }
    }

    // Освобождаем документ и освобождаем ресурсы libxml2
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

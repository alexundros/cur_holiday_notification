#include "xml_utils.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_result_list(result_list_t* rl) {
    rl->count = 0;
    rl->capacity = 16;
    rl->items = (result_item_t*)malloc(sizeof(result_item_t) * rl->capacity);
}

void append_result(result_list_t* rl, const char* code, const char* date) {
    if (rl->count >= rl->capacity) {
        rl->capacity *= 2;
        rl->items = (result_item_t*)realloc(rl->items, sizeof(result_item_t) * rl->capacity);
    }
    strncpy(rl->items[rl->count].code, code, sizeof(rl->items[rl->count].code) - 1);
    rl->items[rl->count].code[sizeof(rl->items[rl->count].code) - 1] = '\0';
    strncpy(rl->items[rl->count].date, date, sizeof(rl->items[rl->count].date) - 1);
    rl->items[rl->count].date[sizeof(rl->items[rl->count].date) - 1] = '\0';
    rl->count++;
}

void free_result_list(result_list_t* rl) {
    if (rl->items) {
        free(rl->items);
        rl->items = NULL;
    }
    rl->count = 0;
    rl->capacity = 0;
}

int process_xml(const char* xml_path, const config_t* config, result_list_t* rl) {
    xmlInitParser();
    xmlDocPtr doc = xmlReadFile(xml_path, NULL, XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    if (doc == NULL) return 1;

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == NULL || xmlStrcmp(root->name, (const xmlChar*)"MICEX_DOC") != 0) {
        xmlFreeDoc(doc);
        return 2;
    }

    for (xmlNodePtr cux = root->children; cux; cux = cux->next) {
        if (cux->type != XML_ELEMENT_NODE || xmlStrcmp(cux->name, (const xmlChar*)"CUX50") != 0) continue;

        xmlChar* attr_date = xmlGetProp(cux, (const xmlChar*)"ReportDate");
        if (attr_date == NULL) continue;

        char report_date[32];
        strncpy(report_date, (const char*)attr_date, sizeof(report_date) - 1);
        report_date[sizeof(report_date) - 1] = '\0';
        xmlFree(attr_date);

        bool found_in_this_cux = false;

        if (config->features.HDay) {
            for (xmlNodePtr grp = cux->children; grp; grp = grp->next) {
                if (grp->type != XML_ELEMENT_NODE || xmlStrcmp(grp->name, (const xmlChar*)"GROUP") != 0) continue;

                xmlChar* tg = xmlGetProp(grp, (const xmlChar*)"TradeGroup");
                if (tg) {
                    if (xmlStrcmp(tg, (const xmlChar*)"H") == 0) {
                        append_result(rl, "HDay", report_date);
                        xmlFree(tg);
                        found_in_this_cux = true;
                        break;
                    }
                    xmlFree(tg);
                }
            }
            if (found_in_this_cux) continue;
        }

        for (xmlNodePtr grp = cux->children; grp; grp = grp->next) {
            if (grp->type != XML_ELEMENT_NODE || xmlStrcmp(grp->name, (const xmlChar*)"GROUP") != 0) continue;

            for (xmlNodePtr sec = grp->children; sec; sec = sec->next) {
                if (sec->type != XML_ELEMENT_NODE || xmlStrcmp(sec->name, (const xmlChar*)"SECURITY") != 0) continue;

                xmlChar* ssn = xmlGetProp(sec, (const xmlChar*)"SecShortName");
                if (ssn) {
                    for (int i = 0; i < config->items.count; i++) {
                        if (strcmp((const char*)ssn, config->items.list[i].key) == 0) {
                            append_result(rl, config->items.list[i].value, report_date);
                            found_in_this_cux = true;
                            break;
                        }
                    }
                    xmlFree(ssn);
                    if (found_in_this_cux) break;
                }
            }
            if (found_in_this_cux) break;
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return 0;
}

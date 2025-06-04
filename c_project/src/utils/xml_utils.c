#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "xml_utils.h"

void init_result_list(ResultList *rl)
{
    rl->count = 0;
    rl->capacity = 16;
    rl->items = (ResultItem *)malloc(sizeof(ResultItem) * rl->capacity);
}

void append_result(ResultList *rl, const char *code, const char *date)
{
    if (rl->count >= rl->capacity)
    {
        rl->capacity *= 2;
        rl->items = (ResultItem *)realloc(rl->items, sizeof(ResultItem) * rl->capacity);
    }
    strncpy(rl->items[rl->count].code, code, sizeof(rl->items[rl->count].code) - 1);
    rl->items[rl->count].code[sizeof(rl->items[rl->count].code) - 1] = '\0';
    strncpy(rl->items[rl->count].date, date, sizeof(rl->items[rl->count].date) - 1);
    rl->items[rl->count].date[sizeof(rl->items[rl->count].date) - 1] = '\0';
    rl->count++;
}

void free_result_list(ResultList *rl)
{
    if (rl->items)
    {
        free(rl->items);
        rl->items = NULL;
    }
    rl->count = 0;
    rl->capacity = 0;
}

int process_xml(const char *xml_path, const config_t *config, ResultList *rl)
{
    xmlInitParser();

    xmlDocPtr doc = xmlReadFile(xml_path, NULL, XML_PARSE_NOWARNING | XML_PARSE_NOERROR);
    if (doc == NULL)
    {
        return -1;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == NULL || xmlStrcmp(root->name, (const xmlChar *)"MICEX_DOC") != 0)
    {
        xmlFreeDoc(doc);
        return -2;
    }

    for (xmlNodePtr cux = root->children; cux; cux = cux->next)
    {
        if (cux->type != XML_ELEMENT_NODE)
            continue;
        if (xmlStrcmp(cux->name, (const xmlChar *)"CUX50") != 0)
            continue;

        xmlChar *attrDate = xmlGetProp(cux, (const xmlChar *)"ReportDate");
        if (attrDate == NULL)
        {
            continue;
        }
        char reportDate[32];
        strncpy(reportDate, (const char *)attrDate, sizeof(reportDate) - 1);
        reportDate[sizeof(reportDate) - 1] = '\0';
        xmlFree(attrDate);

        bool foundInThisCux = false;

        if (config->features.HDay)
        {
            for (xmlNodePtr grp = cux->children; grp; grp = grp->next)
            {
                if (grp->type != XML_ELEMENT_NODE)
                    continue;
                if (xmlStrcmp(grp->name, (const xmlChar *)"GROUP") != 0)
                    continue;

                xmlChar *tg = xmlGetProp(grp, (const xmlChar *)"TradeGroup");
                if (tg && xmlStrcmp(tg, (const xmlChar *)"H") == 0)
                {
                    append_result(rl, "HDay", reportDate);
                    xmlFree(tg);
                    foundInThisCux = true;
                    break;
                }
                if (tg)
                    xmlFree(tg);
            }
            if (foundInThisCux)
            {
                continue;
            }
        }

        for (int i = 0; i < config->items.count; i++)
        {
            const char *key = config->items.list[i].key;
            const char *val = config->items.list[i].value;

            bool itemMatched = false;
            for (xmlNodePtr grp = cux->children; grp; grp = grp->next)
            {
                if (grp->type != XML_ELEMENT_NODE)
                    continue;
                if (xmlStrcmp(grp->name, (const xmlChar *)"GROUP") != 0)
                    continue;

                for (xmlNodePtr sec = grp->children; sec; sec = sec->next)
                {
                    if (sec->type != XML_ELEMENT_NODE)
                        continue;
                    if (xmlStrcmp(sec->name, (const xmlChar *)"SECURITY") != 0)
                        continue;

                    xmlChar *ssn = xmlGetProp(sec, (const xmlChar *)"SecShortName");
                    if (ssn && strcmp((const char *)ssn, key) == 0)
                    {
                        append_result(rl, val, reportDate);
                        xmlFree(ssn);
                        itemMatched = true;
                        break;
                    }
                    if (ssn)
                        xmlFree(ssn);
                }
                if (itemMatched)
                    break;
            }
            if (itemMatched)
            {
                continue;
            }
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

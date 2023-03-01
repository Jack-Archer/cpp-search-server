#include "document.h"

Document::Document (int id_doc, double rel, int rate, DocumentStatus stat)
    {
        Document::id = id_doc;
        Document::relevance = rel;
        Document::rating = rate;
    }
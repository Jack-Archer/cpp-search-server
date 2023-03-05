#include "document.h"
#include <iostream>

Document::Document (int id_doc, double rel, int rate, DocumentStatus stat)
    {
        Document::id = id_doc;
        Document::relevance = rel;
        Document::rating = rate;
    }

std::ostream& operator<<(std::ostream& out, const Document& doc) {
    out << "{ document_id = " << doc.id << ",";
    out << " relevance = " << doc.relevance << ",";
    out << " rating = " << doc.rating << " }";
    return out;
}

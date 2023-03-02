#pragma once

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
    NULL_STATUS
};


struct Document
{
    Document () = default;

    Document (int id_doc, double rel, int rate, DocumentStatus stat = DocumentStatus::NULL_STATUS);
    
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    DocumentStatus status = DocumentStatus::NULL_STATUS;
};

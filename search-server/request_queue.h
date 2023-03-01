#pragma once
#include <string>
#include <vector>
#include <deque>
#include "search_server.h"

class RequestQueue {
public:
    RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        QueryResult temp_qr;
        temp_qr.found_ = SearchServer_.FindTopDocuments(raw_query,document_predicate);
        temp_qr.query_ = raw_query;
        process_result(temp_qr);
        return temp_qr.found_;

    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;
private:
    struct QueryResult {
        std::string query_;
        std::vector<Document> found_;
        bool result_;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer &SearchServer_;
    int count_ = 0;

    void process_result(QueryResult &temp_qr);
};
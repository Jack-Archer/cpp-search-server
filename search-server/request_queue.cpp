#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server) : SearchServer_(search_server){}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    RequestQueue::QueryResult temp_qr;
    temp_qr.found_ = RequestQueue::SearchServer_.FindTopDocuments(raw_query,status);
    temp_qr.query_ = raw_query;
    RequestQueue::process_result(temp_qr);
    return temp_qr.found_;
}
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    RequestQueue::QueryResult temp_qr;
    temp_qr.found_ = SearchServer_.FindTopDocuments(raw_query);
    temp_qr.query_ = raw_query;
    RequestQueue::process_result(temp_qr);
    return temp_qr.found_;
}


int RequestQueue::GetNoResultRequests() const {
    int count = 0;
    for (auto [q,d,r] :  RequestQueue::requests_) {
        if (r == false) {
            ++count;
        }
    }
    return count;
}

void RequestQueue::process_result(RequestQueue::QueryResult &temp_qr) {
    ++count_;
    if (temp_qr.found_.empty()) {
        temp_qr.result_ = false;
    } else {
        temp_qr.result_ = true;
    }

    if (count_ > min_in_day_ && !requests_.empty()) {
        requests_.pop_front();
    }
    requests_.push_back(temp_qr);
}

#include "process_queries.h"
#include <cassert>
#include <algorithm>
#include <execution>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> result(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(), result.begin(),[&search_server](const std::string& query){return search_server.FindTopDocuments(query);});
    return result;
}

std::list <Document> ProcessQueriesJoined (const SearchServer& search_server, const std::vector<std::string>& queries) {
    const auto vec = ProcessQueries(search_server,queries);
    std::list<Document> result;
    for(auto& doc_vec : vec) {
        for(auto& doc : doc_vec) {
            result.push_back(doc);
        }
    }
    return result;
}
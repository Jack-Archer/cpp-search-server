#include "remove_duplicates.h"
#include <set>
#include <map>
#include <string>
#include <iostream>


void RemoveDuplicates(SearchServer& search_server) {
    std::set<int> docs_to_remove;
    auto it = search_server.GetIdToWords().begin();
    while (it != search_server.GetIdToWords().end()) {
        auto it2 = it;
        ++it2;
        while (it2 != search_server.GetIdToWords().end()) {
            if (it->second == it2->second) {
                docs_to_remove.insert(it2->first);
            }
            ++it2;
        }
        ++it;
    }
    for(auto &it : docs_to_remove) {
        std::cout << "Found duplicate document id " << it << std::endl;
        search_server.RemoveDocument(it);
    }
}
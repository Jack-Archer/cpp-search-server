#include "remove_duplicates.h"
#include <set>
#include <map>
#include <string>
#include <iostream>


/*void RemoveDuplicates(SearchServer& search_server) {
    
  std::set<int> ids;

    for (auto it = search_server.GetIdToWords().begin() ; it != search_server.GetIdToWords().end(); ++it) {
        for (auto iter = ++it ; iter != search_server.GetIdToWords().end(); ++iter) {
            if (it->second == iter->second) {
                ids.insert(iter->first);
            }
        }
    }
    for (const auto& id : ids) {
        std::cout << "Found duplicate document id " << id << std::endl;
        search_server.RemoveDocument(id);
    }
}*/

/*void RemoveDuplicates(SearchServer& search_server) {
    for (auto it = search_server.GetIdToWords().begin() ; it != search_server.GetIdToWords().end(); std::next(it, 1)) {
        std::cout << "AAAAAAA" << std::endl;
        for (auto iter = ++it ; iter != search_server.GetIdToWords().end();std::next(iter,1)) {

                if (it->second == search_server.GetIdToWords().at(*iter)) {
                    std::cout << "Found duplicate document id " << *iter << std::endl;
                    search_server.RemoveDocument(*iter);

                      std::cout << "AAAAAAA" << std::endl;
            }
        }
    }
}*/

void RemoveDuplicates(SearchServer& search_server) {
    std::set<int> docs_to_remove;
    auto it = search_server.GetIdToWords().begin();
    while (it != search_server.GetIdToWords().end()) {
        auto it2 = it;
        ++it2;
        /*std::cout << it->first << ": ";
        for(auto &itt : it->second) {
            std::cout << itt << " ";
        }
        std::cout << std::endl;*/
        while (it2 != search_server.GetIdToWords().end()) {
            if (it->second == it2->second) {
                //std::cout << it->first << " " << it2->first << std::endl;
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
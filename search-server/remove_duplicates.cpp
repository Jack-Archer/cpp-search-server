#include "remove_duplicates.h"
#include <set>
#include <map>
#include <string>
#include <iostream>


#include "remove_duplicates.h"
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>


void RemoveDuplicates(SearchServer& search_server) {
    std::set<std::set<std::string>> docs;
    std::set<int> docs_to_remove;
    for(auto& [id, set_words] : search_server.GetIdToWords()) {
        if (count(docs.begin(),docs.end(), set_words) != 0) {
            docs_to_remove.insert(id);
        } else docs.insert(set_words);
    }
     for(auto &it : docs_to_remove) {
        std::cout << "Found duplicate document id " << it << std::endl;
        search_server.RemoveDocument(it);
    }
}

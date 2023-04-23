#include "string_processing.h"
#include "search_server.h"
#include <algorithm>
#include <numeric>


std::vector<std::string_view> SplitIntoWords(std::string_view str) {
    std::vector<std::string_view> result;
    str.remove_prefix(std::min(str.size(), str.find_first_not_of(' ')));
 
    while (!str.empty()) {
        
        auto space = str.find(' ');
        result.push_back(space == str.npos ? str.substr(0, str.size()) : str.substr(0, space));
        
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(' ', space)));
    }
    return result;
}


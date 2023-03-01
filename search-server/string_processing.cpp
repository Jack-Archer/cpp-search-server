#include "string_processing.h"
#include "search_server.h"
#include <algorithm>
#include <numeric>
    
std::vector<std::string> SplitIntoWords(const std::string& text)
    {
        std::vector<std::string> words;
        std::string word;
        for (const char c : text)
        {
            if (c == ' ')
            {
                if (!word.empty())
                {
                    words.push_back(word);
                    word.clear();
                }
                else word.clear();
            }
            else word += c;
        }
        if (!word.empty())
        {
            words.push_back(word);
        }

        return words;
    }

bool IsValidWord(const std::string& word)
    {
        return std::none_of(word.begin(), word.end(), [](char c)
        {
            return c >= '\0' && c < ' ';
        });
    }



bool ChekDoubleMinus(const std::string & word) {
    if ((word[0] == '-' && word[1] == '-') ||(word == "-"))
    {
        return false;
    }
    else return true;
}

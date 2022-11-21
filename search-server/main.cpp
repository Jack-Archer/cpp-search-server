#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>


using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}


struct Document {
    int id;
    double relevance;
};


class SearchServer {
    public:

 void AddDocument(int document_id, const string& document) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    ++document_count_;
    double tf_single = 1.0 / words.size();
    for (const string& word : words)
        {
            word_to_document_freqs_[word][document_id] += tf_single;
        }
}

set<string> SetStopWords(const string& text) {
        set<string> stop_words;
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
        return stop_words_;
        }

vector<Document>FindTopDocuments(const string& raw_query) const {
            const Query query_words = ParseQuery(raw_query);
            auto matched_documents = FindAllDocuments(query_words);
            sort(matched_documents.begin(), matched_documents.end(),[](const Document& lhs, const Document& rhs)
     {return lhs.relevance > rhs.relevance;});
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
        }

    private:

    struct Query
    {
        set<string> minus_words_;
        set<string> plus_words_;
    };

    map<string, map<int, double>> word_to_document_freqs_;
    set <string>stop_words_;
    int document_count_ = 0;

Query SplitOnPlusMinus (const set<string>& query_words) const {
    Query q;
    int first_symbol = 0;
    for(const string& word : query_words)
    {
        if(word[first_symbol] == '-')
        {
            q.minus_words_.insert(word.substr(1));
        } else
            {
            q.plus_words_.insert(word);
            }
    }
    return q;
}


Query ParseQuery(const string& text) const {
    set<string> query_words;
    for (const string& word : SplitIntoWordsNoStop(text))
        {
            query_words.insert(word);
        }
    Query q;
    q = SplitOnPlusMinus(query_words);
    return q;
}

vector<string> SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if (stop_words_.count(word) == 0)
            {
            words.push_back(word);
            }
    }
    return words;
}

double GetWordIDF (const string& word) const {
    return log(static_cast<double> (document_count_) / word_to_document_freqs_.at(word).size());
}

 map<int, double> CheckPlusMinusWords (const Query query_words) const {
     map<int, double> document_to_relevance;
         for (const string& word : query_words.plus_words_) {
        if (word_to_document_freqs_.count(word) != 0) {
            int count_match = word_to_document_freqs_.at(word).size();
                if (count_match != 0) {
                    double IDF_word = GetWordIDF(word);
                    map<int, double> ID_with_TF = word_to_document_freqs_.at(word);
                    for (const auto& [id, tf] : ID_with_TF) {
                            document_to_relevance[id] += IDF_word*ID_with_TF[id];
                    }
                }
        }
    }
    for (const string& word : query_words.minus_words_) {
        int count_match = word_to_document_freqs_.count(word);
            if (count_match != 0) {
                for (const auto& [id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(id);
                }
            }
    }

    return document_to_relevance;
}

vector<Document> FindAllDocuments(const Query query_words) const {
    map<int, double> document_to_relevance= CheckPlusMinusWords(query_words);
    vector<Document> matched_documents;
    for (const auto& [id,rel] : document_to_relevance) {
      Document q;
      q.id = id;
      q.relevance = rel;
      matched_documents.push_back(q);
    }
    return matched_documents;
}

};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
     search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {

    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();

    for (const auto& [document_id,relevance] : search_server.FindTopDocuments(query)) {

        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s << endl;
    }

return 0;
}

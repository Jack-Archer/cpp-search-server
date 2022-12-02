#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>
#include <tuple>

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

vector<int> ReadNumbers() {
    string s;
    getline(cin, s);
    int num;
    vector<int> numbers;

    while(cin>>s) {
        cin>>num;
        numbers.push_back(num);
    }

    return numbers;
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
    int rating;
};

 enum class DocumentStatus {
        ACTUAL,
        IRRELEVANT,
        BANNED,
        REMOVED
        };

class SearchServer {
public:

int GetDocumentCount() const {
    return document_count_;
}

 void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    ++document_count_;
    id_to_status_to_rating_.insert({document_id, {status,ComputeAverageRating(ratings)}});

    double tf_single = 1.0 / words.size();
    for (const string& word : words){
            word_to_document_freqs_[word][document_id] += tf_single;
        }
}

tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
    vector<string> tuple_pl_words;
    Query q = ParseQuery(raw_query);
    for(const auto& mw : q.minus_words_) {
            if (word_to_document_freqs_.count(mw) != 0 &&  word_to_document_freqs_.at(mw).count(document_id)!= 0) {
                    return make_tuple(tuple_pl_words,id_to_status_to_rating_.at(document_id).first);
                }
        }

    for(const auto& pw : q.plus_words_) {
            if ( word_to_document_freqs_.count(pw) != 0 && word_to_document_freqs_.at(pw).count(document_id) != 0 ) {
                tuple_pl_words.push_back(pw);
            }
        }

        sort(tuple_pl_words.begin(),tuple_pl_words.end());
        return make_tuple(tuple_pl_words,id_to_status_to_rating_.at(document_id).first);
}

set<string> SetStopWords(const  string& text) {
    for (const string& word : SplitIntoWords(text)) {
        stop_words_.insert(word);
        }
    return stop_words_;
}

template <typename Predicate>
vector<Document> Choose_all_parameters (vector<Document>& match_doc, Predicate predicate) const {
    vector<Document> matched_documents;
    for (const Document& doc : match_doc) {
            int id = doc.id;
            DocumentStatus status = id_to_status_to_rating_.at(id).first;
            int rate = doc.rating;
            if (predicate(doc.id,status,rate)) {
                    matched_documents.push_back(doc);
                }
            else continue;
        }

    return  matched_documents;
}

vector<Document> FindTopDocuments(const string& raw_query) const {
           return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}


template <typename Predicate>
vector<Document>FindTopDocuments(const string& raw_query, Predicate predicate) const {
    const Query query_words = ParseQuery(raw_query);
    vector<Document> match_doc = FindAllDocuments(query_words);

    vector<Document> matched_documents = Choose_all_parameters(match_doc,predicate);

    const double EPSILON = 1e-6;
    sort(matched_documents.begin(), matched_documents.end(),[&EPSILON](const Document& lhs, const Document& rhs)
    { if (abs(lhs.relevance - rhs.relevance)< EPSILON){
         return lhs.rating > rhs.rating;} else {
             return lhs.relevance > rhs.relevance;}});

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT){
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
    return matched_documents;
}

vector<Document> FindTopDocuments(const string& raw_query,DocumentStatus stat) const {
           const auto predicate = [stat](int document_id, DocumentStatus status, int rating) { return status == stat;};
           return FindTopDocuments(raw_query, predicate);
}

private:


struct Query {
    set<string> minus_words_;
    set<string> plus_words_;
};

map <int , pair<DocumentStatus, int>> id_to_status_to_rating_;
map<string, map<int, double>> word_to_document_freqs_;
set <string>stop_words_;

int document_count_ = 0;

static int ComputeAverageRating(const vector<int>& ratings) {
    int size_v = ratings.size();
    int sum = accumulate(ratings.begin(),ratings.end(), 0);
    return (sum / size_v);
}

Query SplitOnPlusMinus (const set<string>& query_words) const {
    Query q;
    for(const string& word : query_words){
            if(word[0] == '-') {
                    q.minus_words_.insert(word.substr(1));
                } else {
                    q.plus_words_.insert(word);
                    }
        }
    return q;
}

Query ParseQuery(const string& text) const {
    set<string> query_words;
    for (const string& word : SplitIntoWordsNoStop(text)) {
            query_words.insert(word);
        }
    Query q;
    q = SplitOnPlusMinus(query_words);
    return q;
}

vector<string> SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
            if (stop_words_.count(word) == 0) {
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
                        const auto& ID_with_TF = word_to_document_freqs_.at(word);
                        for (const auto& [id, tf] : ID_with_TF) {
                                document_to_relevance[id] += IDF_word*ID_with_TF.at(id);
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

// ==================== ��� ������� =========================

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}
int main() {
    SearchServer search_server;
    search_server.SetStopWords("� � ��"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "�������� ��� �������� �����"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "��������� ������� �������"s,         DocumentStatus::BANNED, {9});
    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s)) {
        PrintDocument(document);
    }
    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; })) {
        PrintDocument(document);
    }
    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }
    return 0;
}

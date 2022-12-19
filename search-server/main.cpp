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

string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<int> ReadNumbers()
{
    string s;
    getline(cin, s);
    int num;
    vector<int> numbers;

    while(cin>>s)
    {
        cin>>num;
        numbers.push_back(num);
    }

    return numbers;
}

bool ChekDoubleMinus(const string & word) {
    if ((word[0] == '-' && word[1] == '-') ||(word == "-"s))
    {
        return false;
    }
    else return true;
}

vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;
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

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
    NULL_STATUS
};


struct Document
{
    Document () = default;

    Document (int id_doc, double rel, int rate, DocumentStatus stat = DocumentStatus::NULL_STATUS)
    {
        id = id_doc;
        relevance = rel;
        rating = rate;
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    DocumentStatus status = DocumentStatus::NULL_STATUS;
};


class SearchServer
{
public:

    template <typename ContainerCollection>
    explicit SearchServer(const ContainerCollection& stop_words)
    {
        for (const auto& word : stop_words)
        {
            if(!word.empty() && IsValidWord(word)) {
                stop_words_.insert(word);
            } else throw invalid_argument("Incorrect symbols"s);
        }
    }


    explicit SearchServer(const string& stop_words) : SearchServer(SplitIntoWords(stop_words))
    {

    }


    int GetDocumentCount() const
    {
        return document_count_;
    }

    int GetDocumentId(int index) const
    {
        if (index < 0 || index > document_count_)
        {
            throw out_of_range("Document position is out of range"s);
        }
        else
        {
            return doc_position_.at(index);
        }
    }

     void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        if (document_id <0 || id_to_all_parameters_.count(document_id) != 0) {
                throw invalid_argument("ID less than zero or dublicate exist"s); }

        const vector<string> words = SplitIntoWordsNoStop(document);

        for (const auto& w : words) {
            if (!IsValidWord(w)) { throw invalid_argument("Text of document include incorrect symbols"s);}
        }
        doc_position_.insert({document_count_,document_id});
        ++document_count_;
        Document q;
        q.id = document_id;
        q.status = status;
        q.rating = ComputeAverageRating(ratings);

        id_to_all_parameters_.insert({document_id, q});

        double tf_single = 1.0 / words.size();
        for (const string& word : words)
        {
            word_to_document_freqs_[word][document_id] += tf_single;
        }
    }

   tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        for (const auto& word : SplitIntoWords(raw_query)) {
            if (!ChekDoubleMinus(word)) {throw invalid_argument("Query include word with (--) or have no word after (-) "s);}
            if (!IsValidWord(word)) {throw invalid_argument("Incorrect symbols in document"s);}
        }
        vector<string> tuple_pl_words;
        Query q = ParseQuery(raw_query);
        for(const auto& mw : q.minus_words_)
        {
            if (word_to_document_freqs_.count(mw) != 0 &&  word_to_document_freqs_.at(mw).count(document_id)!= 0)
            {
                return make_tuple (tuple_pl_words,id_to_all_parameters_.at(document_id).status);
            }
        }


        for(const auto& pw : q.plus_words_)
        {
            if ( word_to_document_freqs_.count(pw) != 0 && word_to_document_freqs_.at(pw).count(document_id) != 0 )
            {
                tuple_pl_words.push_back(pw);
            }
        }

        sort(tuple_pl_words.begin(),tuple_pl_words.end());
        return make_tuple(tuple_pl_words,id_to_all_parameters_.at(document_id).status);
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }


    template <typename DocumentPredicate>
   vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {
        for (const auto& word : SplitIntoWords(raw_query)) {
            if (!ChekDoubleMinus(word)) {throw invalid_argument("Query include word with (--) or have no word after (-) "s);}
            if (!IsValidWord(word)) {throw invalid_argument("Incorrect symbols in document"s);}
        }
        const Query query_words = ParseQuery(raw_query);
        auto result = FindAllDocuments(query_words,document_predicate);
        const double EPSILON = 1e-6;
        sort(result.begin(), result.end(),[&EPSILON](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance)< EPSILON) {
                return lhs.rating > rhs.rating;
            } else {
                return lhs.relevance > rhs.relevance;
            } });
            if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
                result.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return result;
    }

     vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        const auto predicate = [status](int document_id, DocumentStatus stat, int rating)
        {
            return stat == status;
        };
        return FindTopDocuments(raw_query, predicate);
    }

private:


    struct Query
    {
        set<string> minus_words_;
        set<string> plus_words_;
    };

    map <int, Document> id_to_all_parameters_;
    map<string, map<int, double>> word_to_document_freqs_;
    set <string>stop_words_;
    map<int, int>doc_position_;

    int document_count_ = 0;

    static bool IsValidWord(const string& word)
    {
        return none_of(word.begin(), word.end(), [](char c)
        {
            return c >= '\0' && c < ' ';
        });
    }

    static int ComputeAverageRating(const vector<int>& ratings)
    {
        int size_v = ratings.size();
        int sum = accumulate(ratings.begin(),ratings.end(), 0);
        return (sum / size_v);
    }

    Query SplitOnPlusMinus (const set<string>& query_words) const
    {
        Query q;
        for(const string& word : query_words)
        {
            if(word[0] == '-')
            {
                q.minus_words_.insert(word.substr(1));
            }
            else
            {
                q.plus_words_.insert(word);
            }
        }
        return q;
    }

    Query ParseQuery(const string& text) const
    {
        set<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text))
        {
            query_words.insert(word);
        }
        Query q;
        q = SplitOnPlusMinus(query_words);
        return q;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const
    {
        vector<string> words;
        for (const string& word : SplitIntoWords(text))
        {
            if (stop_words_.count(word) == 0)
            {
                words.push_back(word);
            }
        }
        return words;
    }

    double GetWordIDF (const string& word) const
    {
        return log(static_cast<double> (document_count_) / word_to_document_freqs_.at(word).size());
    }

    map<int, double> CheckPlusMinusWords (const Query query_words) const
    {
        map<int, double> document_to_relevance;
        for (const string& word : query_words.plus_words_)
        {
            if (word_to_document_freqs_.count(word) != 0)
            {
                int count_match = word_to_document_freqs_.at(word).size();
                if (count_match != 0)
                {
                    double IDF_word = GetWordIDF(word);
                    const auto& ID_with_TF = word_to_document_freqs_.at(word);
                    for (const auto& [id, tf] : ID_with_TF)
                    {
                        document_to_relevance[id] += IDF_word*ID_with_TF.at(id);
                    }
                }
            }
        }
        for (const string& word : query_words.minus_words_)
        {
            int count_match = word_to_document_freqs_.count(word);
            if (count_match != 0)
            {
                for (const auto& [id, tf] : word_to_document_freqs_.at(word))
                {
                    document_to_relevance.erase(id);
                }
            }
        }

        return document_to_relevance;
    }
    template <typename Predicate>
    vector<Document> FindAllDocuments(const Query query_words, Predicate predicate) const
    {
        map<int, double> document_to_relevance = CheckPlusMinusWords(query_words);
        vector<Document> matched_documents;
        for (const auto& [id,rel] : document_to_relevance)
        {
            Document q;
            q.id = id;
            q.rating = id_to_all_parameters_.at(id).rating;
            q.status = id_to_all_parameters_.at(id).status;
            q.relevance = rel;
            matched_documents.push_back(q);
        }
        vector<Document>match_doc;
        for (const Document& doc : matched_documents)
        {
            if (predicate(doc.id,doc.status,doc.rating))
            {
                match_doc.push_back(doc);
            }
            else continue;
        }

        return match_doc;
    }

};

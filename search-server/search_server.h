#pragma once
#include "string_processing.h"
#include "document.h"
#include <tuple>
#include <set>
#include <map>
#include <cmath>
#include <stdexcept>



const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {

public:

    template <typename ContainerCollection>
    explicit SearchServer(const ContainerCollection& stop_words);

    explicit SearchServer(const std::string& stop_words);

    int GetDocumentCount() const;

    std::map<int,std::set<std::string>>& GetIdToWords();

     const std::set<int>::iterator begin();

     const std::set<int>::iterator end();

    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);


    bool ChekDoubleMinus(const std::string & word) const;

    bool IsValidWord(const std::string& word) const;

    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    int ComputeAverageRating(const std::vector<int>& ratings);

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;


    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

template <typename DocumentPredicate>
std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const;

std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;


    private:


    struct Query
    {
        std::set<std::string> minus_words_;
        std::set<std::string> plus_words_;
    };

    Query ParseQuery(const std::string& text) const;

    Query SplitOnPlusMinus (const std::set<std::string>& query_words) const;


    std::map <int, Document> id_to_all_parameters_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::set <std::string> stop_words_;
    std::set<int> all_docs_ids_;
    std::map<int, std::map<std::string, double>> word_freq_;
    std::map<int,std::set<std::string>> id_doc_words_;


    int document_count_ = 0;

    std::map<int, double> CheckPlusMinusWords (const Query query_words) const;

    template <typename Predicate>
    std::vector<Document> FindAllDocuments(const Query query_words, Predicate predicate) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    double GetWordIDF (const std::string& word) const ;

};


template <typename ContainerCollection>
SearchServer::SearchServer(const ContainerCollection& stop_words){
        for (const auto& word : stop_words)
        {
            if(!word.empty() && IsValidWord(word)) {
                stop_words_.insert(word);
            } else throw std::invalid_argument("Incorrect symbols at stop word : " + word);
        }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {

    const Query query_words = ParseQuery(raw_query);
    auto result = FindAllDocuments(query_words,document_predicate);
    const double EPSILON = 1e-6;
    sort(result.begin(), result.end(),[&EPSILON](const Document& lhs, const Document& rhs) {
        if (std::abs(lhs.relevance - rhs.relevance)< EPSILON) {
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        } });
        if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
            result.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return result;
}

template <typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query query_words, Predicate predicate) const
{
    std::map<int, double> document_to_relevance = CheckPlusMinusWords(query_words);
    std::vector<Document> matched_documents;
    for (const auto& [id,rel] : document_to_relevance)
        {
            Document q;
            q.id = id;
            q.rating = id_to_all_parameters_.at(id).rating;
            q.status = id_to_all_parameters_.at(id).status;
            q.relevance = rel;
            matched_documents.push_back(q);
        }
    std::vector<Document>match_doc;
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



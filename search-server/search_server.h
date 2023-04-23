#pragma once

#include "string_processing.h"
#include "concurrent_map.h"
#include "document.h"
#include <tuple>
#include <set>
#include <map>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <execution>
#include <vector>
#include <list>
#include <thread>
#include <future>



const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer {

public:

    template <typename ContainerCollection>
    explicit SearchServer(const ContainerCollection& stop_words);

    explicit SearchServer(const std::string& stop_words);
    
    explicit SearchServer(const std::string_view& stop_words);

    int GetDocumentCount() const;

    std::map<int,std::set<std::string_view>>& GetIdToWords();

     const std::set<int>::iterator begin();

     const std::set<int>::iterator end();

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);
    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);


    bool ChekDoubleMinus(std::string_view word) const;

    bool IsValidWord(std::string_view word) const;

    void AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings);

    int ComputeAverageRating(const std::vector<int>& ratings);

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view& raw_query, int document_id) const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, const std::string_view& raw_query, int document_id) const;
    
     std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, const std::string_view& raw_query, int document_id) const;

std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

template <typename DocumentPredicate>
std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentPredicate predicate) const;

std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status) const;

template <typename Policy>
std::vector<Document> FindTopDocuments(const Policy& policy, std::string_view raw_query) const ;

template <typename Policy, typename DocumentPredicate>
std::vector<Document> FindTopDocuments(const Policy& policy, std::string_view raw_query, DocumentPredicate predicate) const;

template <typename Policy>    
std::vector<Document> FindTopDocuments(const Policy& policy, std::string_view raw_query, DocumentStatus status) const;
    

    private:


    struct Query
    {
        std::vector<std::string_view> minus_words_;
        std::vector<std::string_view> plus_words_;
    };

    Query ParseQuery(std::string_view text, bool flag) const;



    std::map <int, Document> id_to_all_parameters_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::set <std::string, std::less<>> stop_words_;
    std::set<int> all_docs_ids_;
    std::map<int, std::map<std::string_view, double>> word_freq_;
    std::map<int,std::set<std::string_view>> id_doc_words_;
    std::map<int, std::list<std::string>> all_document_words_;


    int document_count_ = 0;
    
template <typename Policy>
std::map<int, double> CheckPlusMinusWords (const Policy& policy, const Query query_words) const; 

template <typename Policy, typename Predicate>
std::vector<Document> FindAllDocuments(const Policy& policy, const Query query_words, Predicate predicate) const;  
    
    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view& text) const;

    double GetWordIDF (std::string_view word) const ;

};


template <typename ContainerCollection>
SearchServer::SearchServer(const ContainerCollection& stop_words){
        for (const auto& word : stop_words)
        {
            if(!word.empty() && IsValidWord(word)) {
                stop_words_.insert(std::string{word});
            } else throw std::invalid_argument("Incorrect symbols at stop word : " + std::string{word});
        }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentPredicate predicate) const {
   return SearchServer::FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template <typename Policy>
std::vector<Document> SearchServer::FindTopDocuments(const Policy& policy, std::string_view raw_query) const {
    return SearchServer::FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename Policy>    
std::vector<Document> SearchServer::FindTopDocuments(const Policy& policy, std::string_view raw_query, DocumentStatus status) const {
     const auto predicate = [status](int document_id, DocumentStatus stat, int rating)
    {
        return stat == status;
    };
    return SearchServer::FindTopDocuments(policy, raw_query, predicate);
}


template <typename Policy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const Policy& policy, std::string_view raw_query, DocumentPredicate predicate) const {
    const SearchServer::Query query_words = ParseQuery(raw_query, true);
    auto result = SearchServer::FindAllDocuments(policy, query_words, predicate);
    const double EPSILON = 1e-6;
    std::sort(policy, result.begin(), result.end(),[&EPSILON](const Document& lhs, const Document& rhs) {
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

template <typename Policy, typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const Policy& policy, const Query query_words, Predicate predicate) const
{
    std::map<int, double> document_to_relevance = CheckPlusMinusWords(policy, query_words);
    std::vector<Document> matched_documents;
    std::for_each(policy, document_to_relevance.begin(), document_to_relevance.end(), [this, &matched_documents](auto& pair) {
            Document q;
            q.id = pair.first;
            q.rating = id_to_all_parameters_.at(pair.first).rating;
            q.status = id_to_all_parameters_.at(pair.first).status;
            q.relevance = pair.second;
            matched_documents.push_back(q);
    });
    
    std::vector<Document>match_doc;
    std::for_each(std::execution::seq, matched_documents.begin(), matched_documents.end(), [this, &match_doc, predicate](auto& doc){
        if (predicate(doc.id,doc.status,doc.rating))
        {
           match_doc.push_back(doc);
        }
        else return;
    });
    return match_doc;
}


template <typename Policy>
std::map<int, double> SearchServer::CheckPlusMinusWords (const Policy& policy, const Query query_words) const {
  ConcurrentMap <int, double> document_to_relevance(std::thread::hardware_concurrency() - 1);
  
std::for_each(policy, query_words.plus_words_.begin(), query_words.plus_words_.end(), [this, &document_to_relevance] (auto word) { 
  if (word_to_document_freqs_.count(word) == 0) {return;}
         int count_match = SearchServer::word_to_document_freqs_.at(word).size();
            if (count_match != 0)  {
                double IDF_word = GetWordIDF(word);
                const auto& ID_with_TF = word_to_document_freqs_.at(word);
                for (const auto& [id, tf] : ID_with_TF) {
                    document_to_relevance[id].ref_to_value += IDF_word * ID_with_TF.at(id);
                }
            
  }});
  
    
    std::for_each(policy, query_words.minus_words_.begin(), query_words.minus_words_.end(),[policy, this, &document_to_relevance](auto word){
    if (SearchServer::word_to_document_freqs_.count(word) == 0) {return;}
        int count_match = SearchServer::word_to_document_freqs_.at(word).size();
            if (count_match != 0)  {
        std::for_each(policy, word_to_document_freqs_.at(word).begin(), word_to_document_freqs_.at(word).end(),[&document_to_relevance, this](auto& pair){
             document_to_relevance.Erase(pair.first);
        });
            }
        
    });
    return document_to_relevance.BuildOrdinaryMap();
}
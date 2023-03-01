#include <numeric>
#include <algorithm>
#include <stdexcept>
#include "string_processing.h"
#include "search_server.h"

SearchServer::SearchServer(const std::string& stop_words) : SearchServer::SearchServer(SplitIntoWords(stop_words))
    {

    }


int SearchServer::GetDocumentCount() const
    {
        return SearchServer::document_count_;
    }


 int SearchServer::GetDocumentId(int index) const
    {
        if (index < 0 || index > SearchServer::document_count_)
        {
            throw std::out_of_range("Document position is out of range");
        }
        else
        {
            return SearchServer::doc_position_.at(index);
        }
    }

 void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
        if ( document_id <0 ) {
                throw std::invalid_argument("ID less than zero");
        }
        if ( SearchServer::id_to_all_parameters_.count(document_id) != 0 ) {
             throw std::invalid_argument("ID of document already exists");
        }

        const std::vector<std::string> words = SplitIntoWordsNoStop(document);

        for (const auto& w : words) {
            if (!IsValidWord(w)) { throw std::invalid_argument("Text of document include incorrect symbols");}
        }
        SearchServer::doc_position_.insert({SearchServer::document_count_,document_id});
        ++SearchServer::document_count_;
        Document q;
        q.id = document_id;
        q.status = status;
        q.rating = ComputeAverageRating(ratings);

        SearchServer::id_to_all_parameters_.insert({document_id, q});

        double tf_single = 1.0 / words.size();
        for (const std::string& word : words)
        {
            SearchServer::word_to_document_freqs_[word][document_id] += tf_single;
        }
    }


int SearchServer::ComputeAverageRating(const std::vector<int>& ratings)
    {
        int size_v = ratings.size();
        int sum = accumulate(ratings.begin(),ratings.end(), 0);
        return (sum / size_v);
    }


std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {

        std::vector<std::string> tuple_pl_words;
        SearchServer::Query q = SearchServer::ParseQuery(raw_query);
        for(const auto& mw : q.minus_words_)
        {
            if (SearchServer::word_to_document_freqs_.count(mw) != 0 &&  SearchServer::word_to_document_freqs_.at(mw).count(document_id)!= 0)
            {
                return std::make_tuple (tuple_pl_words,SearchServer::id_to_all_parameters_.at(document_id).status);
            }
        }


        for(const auto& pw : q.plus_words_)
        {
            if ( SearchServer::word_to_document_freqs_.count(pw) != 0 && SearchServer::word_to_document_freqs_.at(pw).count(document_id) != 0 )
            {
                tuple_pl_words.push_back(pw);
            }
        }

        std::sort(tuple_pl_words.begin(),tuple_pl_words.end());
        return std::make_tuple(tuple_pl_words,SearchServer::id_to_all_parameters_.at(document_id).status);
    }

 
SearchServer::Query SearchServer::ParseQuery(const std::string& text) const
    {
        std::set<std::string> query_words;
        for (const std::string& word : SearchServer::SplitIntoWordsNoStop(text))
        {
            if (!ChekDoubleMinus(word)) {throw std::invalid_argument("Query include word with (--) or have no word after (-) ");}
            if (!IsValidWord(word)) {throw std::invalid_argument("Incorrect symbols in document");}
            query_words.insert(word);
        }
        SearchServer::Query q;
        q = SplitOnPlusMinus(query_words);
        return q;
    }

SearchServer::Query SearchServer::SplitOnPlusMinus (const std::set<std::string>& query_words) const
    {
        SearchServer::Query q;
        for(const std::string& word : query_words)
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


std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
        return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }


 std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
        const auto predicate = [status](int document_id, DocumentStatus stat, int rating)
        {
            return stat == status;
        };
        return SearchServer::FindTopDocuments(raw_query, predicate);
    }


std::map<int, double> SearchServer::CheckPlusMinusWords (const Query query_words) const
    {
        std::map<int, double> document_to_relevance;
        for (const std::string& word : query_words.plus_words_)
        {
            if (SearchServer::word_to_document_freqs_.count(word) != 0)
            {
                int count_match = SearchServer::word_to_document_freqs_.at(word).size();
                if (count_match != 0)
                {
                    double IDF_word = GetWordIDF(word);
                    const auto& ID_with_TF = SearchServer::word_to_document_freqs_.at(word);
                    for (const auto& [id, tf] : ID_with_TF)
                    {
                        document_to_relevance[id] += IDF_word*ID_with_TF.at(id);
                    }
                }
            }
        }
        for (const std::string& word : query_words.minus_words_)
        {
            int count_match = SearchServer::word_to_document_freqs_.count(word);
            if (count_match != 0)
            {
                for (const auto& [id, tf] : SearchServer::word_to_document_freqs_.at(word))
                {
                    document_to_relevance.erase(id);
                }
            }
        }

        return document_to_relevance;
    }


std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
        std::vector<std::string> words;
        for (const std::string& word : SplitIntoWords(text))
        {
            if (stop_words_.count(word) == 0)
            {
                words.push_back(word);
            }
        }
        return words;
    }


 double SearchServer::GetWordIDF (const std::string& word) const
    {
        return log(static_cast<double> (document_count_) / word_to_document_freqs_.at(word).size());
    }

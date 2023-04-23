#include <numeric>
#include <algorithm>
#include <stdexcept>
#include "string_processing.h"
#include "search_server.h"
#include <set>
#include <execution>

SearchServer::SearchServer(const std::string& stop_words) : SearchServer::SearchServer(SplitIntoWords(stop_words)){}

SearchServer::SearchServer(const std::string_view& stop_words) : SearchServer::SearchServer(SplitIntoWords(stop_words)){}

int SearchServer::GetDocumentCount() const {
    return SearchServer::document_count_;
}



void SearchServer::AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings) {
    if ( document_id < 0 ) {
            throw std::invalid_argument("ID less than zero");
    }
    if (id_to_all_parameters_.count(document_id) != 0 ) {
         throw std::invalid_argument("ID of document already exists");
    }

    const std::vector<std::string_view> words = SplitIntoWordsNoStop(document);

    for (const auto& w : words) {
        if (!IsValidWord(w)) { throw std::invalid_argument("Text of document include incorrect symbols");}
    }
    all_docs_ids_.insert(document_id);
    ++SearchServer::document_count_;
    Document q;
    q.id = document_id;
    q.status = status;
    q.rating = ComputeAverageRating(ratings);

    id_to_all_parameters_.insert({document_id, q});

    double tf_single = 1.0 / words.size();
    
    std::set<std::string_view> w;

    for (std::string_view word : words) {
        std::string original{word};
        all_document_words_[document_id].push_back(original);
        word_to_document_freqs_[std::string_view{all_document_words_[document_id].back()}][document_id] += tf_single;
        word_freq_[document_id][std::string_view{all_document_words_[document_id].back()}]+= tf_single;
        w.insert(std::string_view{all_document_words_[document_id].back()});
    }
    id_doc_words_[document_id] = w;
}

const std::set<int>::iterator SearchServer::begin() {
    return all_docs_ids_.begin();
}

const std::set<int>::iterator SearchServer::end() {
    return all_docs_ids_.end();
}

std::map<int,std::set<std::string_view>>& SearchServer::GetIdToWords() {
    return id_doc_words_;
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    return SearchServer::word_freq_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
   if (std::find(all_docs_ids_.begin(), all_docs_ids_.end(), document_id) == all_docs_ids_.end()) {
      return;
  }
  id_to_all_parameters_.erase(document_id);
 
  std::vector<const std::string_view*> word_pointers(word_freq_[document_id].size()); 
  std::transform(word_freq_[document_id].begin(), word_freq_[document_id].end(), word_pointers.begin(), [document_id, this](auto&  str_rate) {return &str_rate.first;});
  std::for_each(word_pointers.begin(), word_pointers.end(),[document_id, this] (auto &word) {
      word_to_document_freqs_.at(*word).erase(document_id);
  });
  word_freq_.erase(document_id);
  all_docs_ids_.erase(document_id);
  id_doc_words_.erase(document_id);
  all_document_words_.erase(document_id);
  --document_count_;
 }


void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
    return SearchServer::RemoveDocument(document_id);
}


void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
  if (std::find(std::execution::par, all_docs_ids_.begin(), all_docs_ids_.end(), document_id) == all_docs_ids_.end()) {
      return;
  }
  id_to_all_parameters_.erase(document_id);
    
  std::vector<const std::string_view*> word_pointers(word_freq_[document_id].size()); 
  std::transform(std::execution::par, word_freq_[document_id].begin(), word_freq_[document_id].end(), word_pointers.begin(), [document_id, this](auto&  str_rate) {return &str_rate.first;});
  std::for_each(std::execution::par, word_pointers.begin(), word_pointers.end(),[document_id, this] (auto &word) {
      word_to_document_freqs_.at(*word).erase(document_id);
  });
  word_freq_.erase(document_id);
  all_docs_ids_.erase(document_id);
  id_doc_words_.erase(document_id);
  all_document_words_.erase(document_id);
  --document_count_;
 }


int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    int size_v = ratings.size();
    int sum = accumulate(ratings.begin(),ratings.end(), 0);
    return (sum / size_v);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, const std::string_view& raw_query, int document_id) const {
    return MatchDocument(raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, const std::string_view& raw_query, int document_id) const {
    
    if (all_docs_ids_.count(document_id) == 0) {
        throw std::out_of_range("Incorrect ID");
    }
    
    SearchServer::Query q = SearchServer::ParseQuery(raw_query, false);

    if (std::any_of(q.minus_words_.begin(), q.minus_words_.end(),[document_id, this](const auto& word){return (word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id)!= 0);})) {
        return std::make_tuple (std::vector<std::string_view>{},SearchServer::id_to_all_parameters_.at(document_id).status);
    }
    
    std::vector<std::string_view> tuple_pl_words(q.plus_words_.size());
    
    auto it = std::copy_if(std::execution::par, q.plus_words_.begin(), q.plus_words_.end(), tuple_pl_words.begin(), [document_id,this](auto& word){
        return (word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id)!= 0);
     });
    
    std::sort(std::execution::par, tuple_pl_words.begin(),it);
    auto it_end = unique(std::execution::par, tuple_pl_words.begin(), it);
    tuple_pl_words.erase(it_end, tuple_pl_words.end());
    return std::make_tuple(tuple_pl_words,SearchServer::id_to_all_parameters_.at(document_id).status);
}


std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view& raw_query, int document_id) const {
    if (all_docs_ids_.count(document_id) == 0) {
        throw std::out_of_range("Incorrect ID");
    } 

    std::vector<std::string_view> tuple_pl_words;
    SearchServer::Query q = SearchServer::ParseQuery(raw_query, true);
    
    if (std::any_of(std::execution::seq, q.minus_words_.begin(), q.minus_words_.end(),[document_id, this](const auto& word){return (word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id)!= 0);})) {
        return std::make_tuple (tuple_pl_words,SearchServer::id_to_all_parameters_.at(document_id).status);
    }
    
    for(const auto& pw : q.plus_words_) {
        if ( SearchServer::word_to_document_freqs_.count(pw) != 0 && SearchServer::word_to_document_freqs_.at(pw).count(document_id) != 0 ) {
            tuple_pl_words.push_back(pw);
        }
    }
     std::sort(std::execution::seq, tuple_pl_words.begin(),tuple_pl_words.end());
    return std::make_tuple(tuple_pl_words,SearchServer::id_to_all_parameters_.at(document_id).status);
}


SearchServer::Query SearchServer::ParseQuery(std::string_view text, bool flag) const {
  SearchServer::Query q;
  for (const std::string_view& word : SearchServer::SplitIntoWordsNoStop(text)) {
        if (!ChekDoubleMinus(word)) {
            throw std::invalid_argument("Query include word with (--) or have no word after (-) ");
        }
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Incorrect symbols in document");
        }
      (word[0] == '-') ? (q.minus_words_.push_back(word.substr(1))) : (q.plus_words_.push_back(word));
    }
    
     if (flag) {
        std::sort(std::execution::seq, q.plus_words_.begin(), q.plus_words_.end());
        auto it = unique(std::execution::seq, q.plus_words_.begin(), q.plus_words_.end());
        q.plus_words_.erase(it, q.plus_words_.end());
    }
    
    return q;
}


std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query)  const {
    return SearchServer::FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}


 std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus status) const {
    return SearchServer::FindTopDocuments(std::execution::seq, raw_query, status);
}



std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view& text) const {
    std::vector<std::string_view> words;
    for (const std::string_view& word : SplitIntoWords(text)) {
        if (stop_words_.count(word) == 0) {
            words.push_back(word);
        }
    }
    return words;
}


double SearchServer::GetWordIDF (std::string_view word) const {
    return log(static_cast<double> (document_count_) / word_to_document_freqs_.at(word).size());
}

bool SearchServer::IsValidWord(std::string_view word) const {
    return std::none_of(word.begin(), word.end(), [](char c) {return c >= '\0' && c < ' ';});
}



bool SearchServer::ChekDoubleMinus(std::string_view word) const {
    if ((word[0] == '-' && word[1] == '-') ||(word == "-")) {
        return false;
    } else return true;
}
#pragma once
#include <iterator>
#include <vector>
#include <ostream>
#include "document.h"

template <typename It>
class IteratorRange {

public:
    It first_;
    It last_;
    size_t size_;
};


template <typename It>
class Paginator {

public:

    Paginator( It begin,  It end, size_t size)
    {
        IteratorRange<It> temp;
        It it_beg = begin;
        It it_end;
        while(it_beg <= end) {
            temp.first_ = it_beg;
            it_end = it_beg + size;
            if(it_end <= end) {
                temp.last_ = it_end;
                temp.size_ = size;
                iter_of_pages_.push_back(temp);
            }
            it_beg += size;
        }
        auto full_range = distance(begin, end);
        auto final_page = full_range % size;
        if (final_page != 0) {
            temp.first_ = end - final_page;
            temp.last_ = end;
            temp.size_ = final_page;
            iter_of_pages_.push_back(temp);
        }
    }

    const auto begin () const {
    return iter_of_pages_.begin();
    }

    const auto end () const {
    return iter_of_pages_.end();
    }

private:
    std::vector <IteratorRange<It>> iter_of_pages_;
};



std::ostream& operator<<(std::ostream& out, const Document& doc) {
    out << "{ document_id = " << doc.id << ",";
    out << " relevance = " << doc.relevance << ",";
    out << " rating = " << doc.rating << " }";
    return out;
}

template <typename It>
std::ostream& operator<<(std::ostream& out, const IteratorRange<It>& c) {
   for (auto it = c.first_; it != c.last_; ++ it) {
    out << *it;
   }
    return out;
}


template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "search_server.h"

using namespace std;


#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__,(hint))

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file, const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),"Stop words must be excluded from documents"s);
    }
}

void TestFindDocumentFromQueryWithWords() {
    int doc_id = 13;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const auto find_doc = server.FindTopDocuments("cat in the city"s);
    ASSERT_EQUAL(find_doc.size(), 1);
    const Document& doc_test = find_doc[0];
    ASSERT_EQUAL(doc_test.id , doc_id);
    }
   }

void TestStopWordsDeleatedFromQuery() {
    int doc_id = 13;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto test_vec = server.FindTopDocuments("in the"s);
        ASSERT(test_vec.empty());
    }
 }


void TestDocsWithoutDocsWithMinusWords() {
    int doc_id = 13;
    const string content = "dog in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("-dog in the city"s).empty());
        ASSERT(server.FindTopDocuments("dog -in the city"s).empty());
        ASSERT(server.FindTopDocuments("dog in -the city"s).empty());
        ASSERT(server.FindTopDocuments("dog in the -city"s).empty());
    }
}

void TestMatchDocumentEmptyOrNot(){
    int doc_id = 13;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        vector<string> testing_vector = {"the"s, "in"s, "cat"s};
        sort(testing_vector.begin(),testing_vector.end());
        const auto test_tuple = server.MatchDocument("the in cat"s,doc_id);
        const auto test_vec = get<0>(test_tuple);
        ASSERT_EQUAL(test_vec, testing_vector);
        const auto test_tuple2 = server.MatchDocument("the in -cat"s,doc_id);
        const auto test_vec2 = get<0>(test_tuple2);
        ASSERT(test_vec2.empty());
    }
}


void TestSortByRelevance() {
    int doc_id = 13;
    const string content = "cat cat cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    int doc_id2 = 4;
    const string content2 = "cat cat in the city"s;
    int doc_id3 = 7;
    const string content3 = "cat in the city"s;
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings);
        vector<Document> test_vector = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL(test_vector.size() , 3);
        ASSERT_EQUAL(test_vector[0].id , 4);
        ASSERT_EQUAL(test_vector[1].id , 7);
        ASSERT_EQUAL(test_vector[2].id , 13);
    }
}

void TestAverageRatingOfDocument() {
    int doc_id = 13;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
     SearchServer server;
     server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
     const auto find_doc = server.FindTopDocuments("city"s);
     ASSERT_EQUAL(find_doc[0].rating , 2);
    }
}

void FilterSearchWithPredicateOfUser() {
    int doc_id = 13;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    int doc_id2 = 4;
    const string content2 = "dog in the city"s;
    const vector<int> ratings2 = {2, 3, 3};
    int doc_id3 = 6;
    const string content3 = "people in the city"s;
    const vector<int> ratings3 = {4, 7, 4};
    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
       server.AddDocument(doc_id2, content2, DocumentStatus::BANNED, ratings2);
       server.AddDocument(doc_id3, content3, DocumentStatus::IRRELEVANT, ratings3);
       vector<Document> test_vector = server.FindTopDocuments("city"s,[]([[maybe_unused]]int document_id, DocumentStatus status, [[maybe_unused]]int rating) { return status == DocumentStatus::BANNED; });
       ASSERT_EQUAL(test_vector.size() , 1);
       ASSERT_EQUAL(test_vector[0].id , doc_id2);
       vector<Document> test_vector2 = server.FindTopDocuments("city"s,[](int document_id, [[maybe_unused]]DocumentStatus status, [[maybe_unused]]int rating) { return document_id == 13; });
       ASSERT_EQUAL(test_vector2.size() , 1);
       ASSERT_EQUAL(test_vector2[0].id , doc_id);
       vector<Document> test_vector3 = server.FindTopDocuments("city"s,[]([[maybe_unused]]int document_id, [[maybe_unused]]DocumentStatus status, int rating) { return rating == 5; });
       ASSERT_EQUAL(test_vector3.size() , 1);
       ASSERT_EQUAL(test_vector3[0].id , doc_id3);
    }
}


void TestSearchDocumentWithStatus() {
    int doc_id = 13;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    int doc_id2 = 4;
    const string content2 = "dog in the city"s;
     int doc_id3 = 7;
    const string content3 = "duck in the city"s;
    int doc_id4 = 666;
    const string content4 = "bear in the city"s;
    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
       server.AddDocument(doc_id2, content2, DocumentStatus::BANNED, ratings);
       server.AddDocument(doc_id3, content3, DocumentStatus::IRRELEVANT, ratings);
       server.AddDocument(doc_id4, content4, DocumentStatus::REMOVED, ratings);
       vector<Document> test_vector = server.FindTopDocuments("city"s,[]([[maybe_unused]]int document_id, DocumentStatus status, [[maybe_unused]]int rating) { return status == DocumentStatus::BANNED;});
       ASSERT_EQUAL(test_vector.size() , 1);
       ASSERT_EQUAL(test_vector[0].id , doc_id2);
       vector<Document> test_vector2 = server.FindTopDocuments("city"s,[]([[maybe_unused]]int document_id, DocumentStatus status, [[maybe_unused]]int rating) { return status == DocumentStatus::ACTUAL;});
       ASSERT_EQUAL(test_vector2.size() , 1);
       ASSERT_EQUAL(test_vector2[0].id , doc_id);
       vector<Document> test_vector3 = server.FindTopDocuments("city"s,[]([[maybe_unused]]int document_id, DocumentStatus status, [[maybe_unused]]int rating) { return status == DocumentStatus::IRRELEVANT;});
       ASSERT_EQUAL(test_vector3.size() , 1);
       ASSERT_EQUAL(test_vector3[0].id , doc_id3);
       vector<Document> test_vector4 = server.FindTopDocuments("city"s,[]([[maybe_unused]]int document_id, DocumentStatus status, [[maybe_unused]]int rating) { return status == DocumentStatus::REMOVED;});
       ASSERT_EQUAL(test_vector4.size() , 1);
       ASSERT_EQUAL(test_vector4[0].id , doc_id4);
    }
}

void TestCorrectWorkOfRelevance() {
    int doc_id = 13;
    const string content = "белый кот и модный ошейник"s;
    const vector<int> ratings = {8, -3}; //0.173287
    int doc_id2 = 4;
    const string content2 = "пушистый кот пушистый хвост"s;
    const vector<int> ratings2 = {7, 2, 7}; //0.866434
    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
       server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
       vector<Document> find_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
       ASSERT_EQUAL(find_docs[0].relevance , 0.34657359027997264311);
       ASSERT_EQUAL(find_docs[0].id , doc_id2);
    }
}

void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    TestFindDocumentFromQueryWithWords();
    TestStopWordsDeleatedFromQuery();
    TestDocsWithoutDocsWithMinusWords();
    TestMatchDocumentEmptyOrNot();
    TestSortByRelevance();
    TestAverageRatingOfDocument();
    FilterSearchWithPredicateOfUser();
    TestSearchDocumentWithStatus();
    TestCorrectWorkOfRelevance();

}

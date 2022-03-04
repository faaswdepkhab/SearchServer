#pragma once
#include <iostream>
#include <string>

void AssertImpl(const bool& t, const std::string& t_str, const std::string& file, const std::string& func, unsigned line, const std::string& hint);

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint)) 

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) {
    using namespace std::string_literals;
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))
   
   template <class F>
void RunTestImpl(F f, const std::string& st_f) {
    f();
    std::cerr << st_f <<" OK" << std::endl;
}

#define RUN_TEST(func) RunTestImpl((func) , #func)



// -------- Начало модульных тестов поисковой системы ----------
void TestAddDocuments();

void TestMatching();

void TestSorting();

void TestRating();

void TestCustomFilter();

void TestStatusFilter();

void TestCalcRelevance();

void TestCalcRelevanceMinusWord();

void TestCalcRelevanceStopWord();


// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent();

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();

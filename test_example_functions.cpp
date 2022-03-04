#include "test_example_functions.h"
#include "search_server.h"
#include "string_processing.h"
#include <execution>
using namespace std;

void AssertImpl(const bool& t, const string& t_str, const string& file, const string& func, unsigned line, const string& hint){
    if (!t){
        cerr << file << "(" << line<< "): " << func << ": ASSERT(" << t_str << ") failed."s;
        if (!hint.empty()){
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))
   
// -------- Начало модульных тестов поисковой системы ----------
void TestAddDocuments() {
    SearchServer server;
    string content;
    const vector<int> ratings = {1, 2, 3};
    server.SetStopWords("in the on off"s);
    
    content="cat on street";
    server.AddDocument(12, content, DocumentStatus::ACTUAL, ratings);
    ASSERT_EQUAL_HINT(server.GetDocumentCount(),1,"Ошибка добавления документа");
    
    content="off on in the";
    server.AddDocument(22, content, DocumentStatus::ACTUAL, ratings);
    ASSERT_EQUAL_HINT(server.GetDocumentCount(),2,"Ошибка добавления документа из стоп слов");
}

void TestMatching() {
    SearchServer server;
    string content;
    const vector<int> ratings = {1, 2, 3};
    server.SetStopWords("in the on off"s);
    
    content="cat on street";
    server.AddDocument(12, content, DocumentStatus::ACTUAL, ratings);
    
    content="dog in home";
    server.AddDocument(22, content, DocumentStatus::ACTUAL, ratings);
    
    ASSERT_EQUAL(get<0>(server.MatchDocument("dog street"s,12)).size(), 1);
    ASSERT_EQUAL(get<0>(server.MatchDocument("dog street"s,22)).size(), 1);
    ASSERT_EQUAL_HINT(get<0>(server.MatchDocument("dog home"s,12)).size(), 0, "Ошибка матчинга: искомых слов нет");
    ASSERT_EQUAL(get<0>(server.MatchDocument("dog home"s,22)).size(), 2);
    ASSERT_HINT(get<0>(server.MatchDocument("-cat home"s,12)).empty(), "Ошибка матчинга: не срабатывает мину-слово");
    ASSERT_EQUAL(get<0>(server.MatchDocument("-cat home"s,22)).size(), 1);
}

void TestSorting() {
    SearchServer search_server("на и"s);
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(10, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(5, "пушистый кот пушистый хвост кот"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(8, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});

    auto Docs=search_server.FindTopDocuments("ухоженный кот"s);
    ASSERT_EQUAL(Docs.size(), 3);
    ASSERT_EQUAL_HINT(Docs[0].id, 8, "Неверный порядок документов");
    ASSERT_EQUAL_HINT(Docs[1].id, 5, "Неверный порядок документов");
    ASSERT_EQUAL_HINT(Docs[2].id, 10, "Неверный порядок документов");
    
}

void TestRating() {
    SearchServer search_server("на и"s);
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(10, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(5, "пушистый кот пушистый хвост кот"s,       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(8, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});

    auto Docs=search_server.FindTopDocuments("ухоженный кот"s);
    ASSERT_EQUAL(Docs.size(), 3);
    ASSERT_EQUAL_HINT(Docs[0].rating, -1, "Ошибка расчёта рейтинга");
    ASSERT_EQUAL_HINT(Docs[1].rating, 5, "Ошибка расчёта рейтинга");
    ASSERT_EQUAL_HINT(Docs[2].rating, 2, "Ошибка расчёта рейтинга");
}

void TestCustomFilter() {
    SearchServer search_server("на и"s);
    
    search_server.SetStopWords(string_view("и в на"s));

    search_server.AddDocument(10, string_view("белый кот и модный ошейник"s),        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(5, string_view("пушистый кот пушистый хвост кот"s),       DocumentStatus::REMOVED, {7, 2, 7});
    search_server.AddDocument(8, string_view("ухоженный пёс выразительные глаза"s), DocumentStatus::BANNED, {5, -12, 2, 1});

    auto Docs=search_server.FindTopDocuments(execution::par, string_view("ухоженный кот"s),
        [](int id,DocumentStatus status,int rating){
            return (rating>0) && (status!=DocumentStatus::ACTUAL) && (id>=0);
        });
    ASSERT_EQUAL(Docs.size(), 1);
    
    Docs=search_server.FindTopDocuments(string_view("ухоженный кот"s),
        [](int id,DocumentStatus status,int rating){
            return (rating>0) && (status==DocumentStatus::REMOVED) && (id>5);
        });
    ASSERT_HINT(Docs.empty(), "Ошибка работы пользовательского фильтра");
}

void TestStatusFilter() {
    SearchServer search_server("на и"s);
    search_server.SetStopWords(string_view("и в на"s));

    search_server.AddDocument(10, string_view("белый кот и модный ошейник"s),        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(5, string_view("пушистый кот пушистый хвост кот"s),       DocumentStatus::REMOVED, {7, 2, 7});
    search_server.AddDocument(8, string_view("ухоженный пёс выразительные глаза"s), DocumentStatus::REMOVED, {5, -12, 2, 1});

    auto Docs=search_server.FindTopDocuments(string_view("ухоженный кот"s),DocumentStatus::BANNED);
    ASSERT(Docs.empty());

    Docs=search_server.FindTopDocuments(string_view("ухоженный кот"s),DocumentStatus::ACTUAL);
    ASSERT_HINT((Docs.size() == 1) && (Docs[0].id==10), "Ошибка работы фильтра по статусу");
    
    Docs=search_server.FindTopDocuments(string_view("ухоженный кот"s),DocumentStatus::REMOVED);
    ASSERT_EQUAL(Docs.size(), 2);
    ASSERT_EQUAL(Docs[0].id, 8);
    ASSERT_EQUAL(Docs[1].id, 5);
}

void TestCalcRelevance() {
    SearchServer search_server("на и"s);
    search_server.SetStopWords(string_view("и в на"s));
    const double TestValues[3] = {0.274653, 0.162186, 0.101366};

    search_server.AddDocument(10, string_view("белый кот и модный ошейник"s),        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(5, string_view("пушистый кот пушистый хвост кот"s),       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(8, string_view("ухоженный пёс выразительные глаза"s), DocumentStatus::ACTUAL, {5, -12, 2, 1});
    
    auto Docs=search_server.FindTopDocuments(string_view("ухоженный кот"s));
    ASSERT_EQUAL(Docs.size(), 3);
    int i;
    for (i=0;i<3;i++){
        ASSERT_HINT(abs(Docs[i].relevance-TestValues[i])<=EPSILON, "Ошибка расчёта релевантности");
    }
}

void TestCalcRelevanceMinusWord() {
    SearchServer search_server("на и"s);
    search_server.SetStopWords("и в на"s);
    const double TestValues[2] = {0.162186, 0.101366};

    search_server.AddDocument(10, string_view("белый кот и модный ошейник"s),        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(5, string_view("пушистый кот пушистый хвост кот"s),       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(8, string_view("ухоженный пёс выразительные глаза"s), DocumentStatus::ACTUAL, {5, -12, 2, 1});
    
    auto Docs=search_server.FindTopDocuments(string_view("ухоженный кот -пёс"s));
    ASSERT_EQUAL_HINT(Docs.size(), 2, "Не срабатывает минус слово");
    int i;
    for (i=0;i<2;i++){
        ASSERT_HINT(abs(Docs[i].relevance-TestValues[i])<=EPSILON, "Ошибка расчёта релевантности при наличии в запросе минус слов");
    }
}

void TestCalcRelevanceStopWord() {
    SearchServer search_server("на и"s);
    search_server.SetStopWords(string_view("и в на"s));
    const double TestValues[3] = {0.346574, 0.277259, 0.173287};

    search_server.AddDocument(10, string_view("белый кот и модный ошейник"s),        DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(5, string_view("пушистый кот пушистый хвост кот"s),       DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(8, string_view("ухоженный пёс выразительные глаза"s), DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(12, string_view("на и в"s), DocumentStatus::ACTUAL, {8, -1, 6, 7});
    
    auto Docs=search_server.FindTopDocuments(string_view("ухоженный кот"s));
    
    ASSERT_EQUAL(Docs.size(), 3);
    int i;
    for (i=0;i<3;i++){
        ASSERT_HINT(abs(Docs[i].relevance-TestValues[i])<=EPSILON, "Ошибка расчёта релевантности при наличии документа из стоп слов");
    }
}


// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string_view content{"cat in the city"};
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, "cat in the city", DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in");
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
        SearchServer server;
        server.SetStopWords(string_view("in the"s));
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments(string_view("in"s)).empty(), "Stop words must be excluded from documents"s);
    }
}

void TestDupliactes() {
    SearchServer search_server(string_view("and with"s));

    search_server.AddDocument(1, string_view("funny pet and nasty rat"s), DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, string_view("funny pet with curly hair"s), DocumentStatus::ACTUAL, {1, 2});

    // дубликат документа 2, будет удалён
    search_server.AddDocument(3, string_view("funny pet with curly hair"s), DocumentStatus::ACTUAL, {1, 2});

    // отличие только в стоп-словах, считаем дубликатом
    search_server.AddDocument(4, string_view("funny pet and curly hair"s), DocumentStatus::ACTUAL, {1, 2});

    // множество слов такое же, считаем дубликатом документа 1
    search_server.AddDocument(5, string_view("funny funny pet and nasty nasty rat"s), DocumentStatus::ACTUAL, {1, 2});

    // добавились новые слова, дубликатом не является
    search_server.AddDocument(6, string_view("funny pet and not very nasty rat"s), DocumentStatus::ACTUAL, {1, 2});

    // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
    search_server.AddDocument(7, string_view("very nasty rat and not very funny pet"s), DocumentStatus::ACTUAL, {1, 2});

    // есть не все слова, не является дубликатом
    search_server.AddDocument(8, string_view("pet with rat and rat and rat"s), DocumentStatus::ACTUAL, {1, 2});

    // слова из разных документов, не является дубликатом
    search_server.AddDocument(9, string_view("nasty rat with curly hair"s), DocumentStatus::ACTUAL, {1, 2});
    
}
/*
Разместите код остальных тестов здесь
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    // Не забудьте вызывать остальные тесты здесь
    RUN_TEST(TestAddDocuments);
    RUN_TEST(TestMatching);
    RUN_TEST(TestSorting);
    RUN_TEST(TestRating);
    RUN_TEST(TestCustomFilter);
    RUN_TEST(TestStatusFilter);
    RUN_TEST(TestCalcRelevance);
    RUN_TEST(TestCalcRelevanceMinusWord);
    RUN_TEST(TestCalcRelevanceStopWord);
    RUN_TEST(TestDupliactes);
}

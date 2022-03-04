#pragma once
#include <vector>
#include <deque>
#include <string>
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        // напишите реализацию
        std::vector<Document> result = search_server_->FindTopDocuments(raw_query, document_predicate);
        ObtainRequest(raw_query, result);
        return result;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;
private:
    struct QueryResult {
        // определите, что должно быть в структуре
        std::string query;
        std::vector<Document> result;
    };
    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    // возможно, здесь вам понадобится что-то ещё
    const SearchServer *search_server_;
    int empty_result_count;
    
    void ObtainRequest(const std::string& raw_query,const std::vector<Document>& result);
};

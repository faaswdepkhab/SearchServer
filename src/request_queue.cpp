//Вставьте сюда своё решение из урока «‎Очередь запросов».‎
#include "request_queue.h"

using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server) {
    // напишите реализацию
    search_server_ = &search_server;
    empty_result_count = 0;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
        // напишите реализацию
    vector<Document> result = search_server_->FindTopDocuments(raw_query, status);
    ObtainRequest(raw_query, result);
    return result;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    // напишите реализацию
    vector<Document> result = search_server_->FindTopDocuments(raw_query);
    ObtainRequest(raw_query, result);
    return result;
}

int RequestQueue::GetNoResultRequests() const {
    // напишите реализацию
    return empty_result_count;
}

void RequestQueue::ObtainRequest(const string& raw_query,const vector<Document>& result){
    if (requests_.size() == sec_in_day_) {
        if (requests_.front().result.empty()) {
            empty_result_count--;
        }
        requests_.pop_front();
    }
        
    if (result.empty()){
        empty_result_count++;
    }

    requests_.push_back({raw_query, result});
}

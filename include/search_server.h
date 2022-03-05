//Вставьте сюда своё решение из урока «‎Очередь запросов».‎
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <execution>
#include <atomic>
	
#include "document.h"
#include "log_duration.h"
#include "string_processing.h"
#include "concurrent_map.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {
public:
    explicit SearchServer() = default;
    
    explicit SearchServer(const std::string &stop_words_text);
    
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words) {
        if (HasBannedCharactersInText(stop_words)) {
            throw std::invalid_argument("Invalid chars in text");
        }
        
        for (auto &s:stop_words) {
            stop_words_.insert(s);
        }
    }
    
    explicit SearchServer(std::string_view stop_words_text);
    
    static int ComputeAverageRating(const std::vector<int>& ratings);
    
    void AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings);
   
    void SetStopWords(std::string_view text);
    
    
    int GetDocumentCount() const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query, int document_id) const;

    template<class ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, std::string_view raw_query, int document_id) const {
        using namespace std;
        if (documents_.count(document_id) == 0) {
            throw out_of_range("No document with id "s + to_string(document_id));
        }

        if (HasBannedCharactersInWord(raw_query)) {
            throw std::invalid_argument("Invalid chars in text");
        }
            
        Query query = ParseQuery(policy, raw_query);
        for (std::string_view s:query.minus_words) {
            if (s.empty()){
                throw std::invalid_argument("Empty minus word");
            }
            if (s[0] == '-'){
                throw std::invalid_argument("Multiply minus char");
            }
        }
    
        tuple<vector<string_view>, DocumentStatus> result;
        get<1>(result) = documents_.at(document_id).status;

        // если в документе есть минус слово возвращаем пустой список
        if (find_if(policy, query.minus_words.cbegin(), query.minus_words.cend(),
            [this, document_id](auto &word) {
                return document_to__word_.at(document_id).count(word) != 0;
            }) != query.minus_words.end()) {
            return result;
        }
        
        mutex guard;
        for_each(policy, query.plus_words.cbegin(), query.plus_words.cend(),
             [this, document_id, &guard, &result](string_view word) {
                if (document_to__word_.at(document_id).count(word) != 0) {
                    // если в документе есть плюс-слово добавляем его (слово) в выдачу
                    guard.lock();
                    get<0>(result).push_back(word_to_document_freqs_.find(word)->first);
                    guard.unlock();
                }
            }
        );
        return result;
    }    
    
    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;
    
    template <class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query) const {
        return FindTopDocuments(policy, raw_query,[](int, DocumentStatus status, int) {
            return status==DocumentStatus::ACTUAL;
        });
    }
    
    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status_filter) const;
    
    template <class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query, DocumentStatus status_filter) const {
        return FindTopDocuments(policy, raw_query,[status_filter](int, DocumentStatus status, int) {
            return status==status_filter;
        });
    }
    
    template <typename FuncFilter>
    std::vector<Document> FindTopDocuments(std::string_view raw_query, FuncFilter func_filter) {
        return FindTopDocuments(std::execution::seq, raw_query, func_filter);
    }
    
    template <typename FuncFilter, class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query, FuncFilter func_filter) const {
        /*
    	LOG_DURATION_STREAM("Operation time", std::cout);   
        */
        if (HasBannedCharactersInWord(raw_query)){
            throw std::invalid_argument("Invalid chars in text");
        }
        
        const Query query = ParseQuery(policy, raw_query);
        
        for (std::string_view s:query.minus_words){
            if (s.empty()){
                throw std::invalid_argument("Empty minus word");
            }
            
            if (s[0] == '-'){
                throw std::invalid_argument("Multiply minus char");
            }
        }
            
        auto matched_documents = FindAllDocuments(policy, query, func_filter);
        
        std::sort(policy, matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                    return lhs.rating > rhs.rating;
                } else {
                    return lhs.relevance > rhs.relevance;
                }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    std::set<int>::const_iterator begin();
    
    std::set<int>::const_iterator end();
    
    const std::map<std::string_view, double> GetWordFrequencies(int document_id) const;
    
    void RemoveDocument(int document_id);
    
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>, std::less<>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_to__word_;
    const std::map<std::string_view, double> empty_document_;
    std::map<int, DocumentData> documents_;
    std::set<int> indexes_docs;
    
    bool IsStopWord(std::string_view word) const;
    
    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;
    
    //static int ComputeAverageRating(const std::vector<int>& ratings);
    
    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(std::string_view text) const;
    
    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };
    
    template<class ExecutionPolicy>
    Query ParseQuery(ExecutionPolicy&& /*policy*/, std::string_view text) const {
        using namespace std;
        
        Query query;
    
        for (string_view word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    
    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string &word) const;

    template <typename FuncFilter>
    std::vector<Document> FindAllDocuments(std::execution::sequenced_policy,const Query& query, FuncFilter func_filter) const {
        using namespace std;
        std::unordered_map<int, double> document_to_relevance;
        for (std::string_view word : query.plus_words) {
            string tmp_word(word);
            if (word_to_document_freqs_.count(tmp_word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(tmp_word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(tmp_word)) {
                if ( func_filter(
                        document_id,
                        documents_.at(document_id).status,
                        documents_.at(document_id).rating
                        ) ) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
        
        for (std::string_view word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            
            for (const auto [document_id, _] : word_to_document_freqs_.at(string(word))) {
                document_to_relevance.erase(document_id);
            }
        }
        
        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({
                document_id,
                relevance,
                documents_.at(document_id).rating
            });
        }
        return matched_documents;
    }
    
    template <typename FuncFilter>
    std::vector<Document> FindAllDocuments(const Query& query, FuncFilter func_filter) const {
        return FindAllDocuments(std::execution::seq, query, func_filter);
    }
    
    template <typename FuncFilter>
    std::vector<Document> FindAllDocuments(std::execution::parallel_policy,const Query& query, FuncFilter func_filter) const {
        using namespace std;
        ConcurrentMap<int, double> document_to_relevance(50);
        for (std::string_view word : query.plus_words) {
            string tmp_word(word);
            if (word_to_document_freqs_.count(tmp_word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(tmp_word);
            mutex guard;
            for_each(std::execution::par, word_to_document_freqs_.at(tmp_word).begin(), 
                word_to_document_freqs_.at(tmp_word).end(),  
                [this, &guard, func_filter, &document_to_relevance, inverse_document_freq](auto &item) {
                if ( func_filter(
                        item.first,
                        documents_.at(item.first).status,
                        documents_.at(item.first).rating
                        ) ) {
                    document_to_relevance[item.first].ref_to_value += item.second * inverse_document_freq;
                }
            });
        }
        
        for (std::string_view word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            
            for (const auto [document_id, _] : word_to_document_freqs_.at(string(word))) {
                document_to_relevance.erase(document_id);
            }
        }
        
        auto res = document_to_relevance.BuildOrdinaryMap();
        std::vector<Document> matched_documents(res.size());
        std::atomic_int i = 0;
        for_each(std::execution::par, res.begin(), res.end(),
            [this, &i, &matched_documents](auto &item) {
                matched_documents[i++] = {
                    item.first,
                    item.second,
                    documents_.at(item.first).rating
                };
            });
        return matched_documents;
    }
    
    bool HasBannedCharactersInWord(const std::string_view s) const;
    
    // Шаблон нужен, так как функция вызывается из конструктора (65-66 строки)
    // где набор стоп-слов передается как шаблон
    template <typename T>
    bool HasBannedCharactersInText(const T &text) const{
        for (auto word:text){
            if (HasBannedCharactersInWord(word)){
                return true;
            }
        }
        return false;
    }
    
};

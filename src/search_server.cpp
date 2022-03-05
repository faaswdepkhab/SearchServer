#include "search_server.h"
#include <numeric>
#include <execution>

using namespace std;

SearchServer::SearchServer(const string &stop_words_text) {
    if (HasBannedCharactersInWord(stop_words_text)){
        throw invalid_argument("Invalid chars in text");
    }
        
    SetStopWords(stop_words_text);
}


SearchServer::SearchServer(string_view stop_words_text) {
    if (HasBannedCharactersInWord(stop_words_text)){
        throw invalid_argument("Invalid chars in text");
    }
        
    SetStopWords(stop_words_text);
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    return reduce(execution::par, ratings.begin(),ratings.end(),0) / static_cast<int>(ratings.size());
}


void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if (document_id<0) {
        throw invalid_argument("Negative ID");
    }
        
    if (documents_.count(document_id)>0) {
        throw invalid_argument("Duplicate ID");
    }
        
    if (HasBannedCharactersInWord(document)) {
        throw invalid_argument("Invalid chars in text");
    }
        
    const vector<string_view> words = SplitIntoWordsNoStop(document);
    double inv_word_count;
    if (words.size()>0) inv_word_count = 1.0 / words.size();
        else inv_word_count = 0.0;
     
    for_each(words.begin(), words.end(),
        [this, document_id, inv_word_count](string_view word) {
            word_to_document_freqs_[string(word)][document_id] += inv_word_count;
            document_to__word_[document_id][word_to_document_freqs_.find(word)->first] += inv_word_count;
        }
    );

    documents_.emplace(document_id, 
        DocumentData {
            ComputeAverageRating(ratings), 
            status
        });
    indexes_docs.insert(document_id);
}

void SearchServer::SetStopWords(string_view text) {
    for (auto &word:SplitIntoWords(text)) {
        stop_words_.insert(string(word));
    }
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}
    
tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

    
bool SearchServer::IsStopWord(string_view word) const {
    return stop_words_.count(string(word)) > 0;
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const {
    vector<string_view> words;
    for (string_view word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    
    return words;
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return {
        text,
        is_minus,
        IsStopWord(text)
    };
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

bool SearchServer::HasBannedCharactersInWord(string_view s) const {
    return count_if(execution::par, s.begin(), s.end(),
        [](char c){
            return (c >= '\0') && (c < ' ');
        }) > 0;
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
    return FindTopDocuments(execution::seq, raw_query);
}
    
vector<Document> SearchServer::FindTopDocuments(const string_view raw_query, DocumentStatus status_filter) const{
    return FindTopDocuments(execution::seq, raw_query, status_filter);
}

set<int>::const_iterator SearchServer::begin() {
    return indexes_docs.cbegin();
}
    
set<int>::const_iterator SearchServer::end() {
    return indexes_docs.cend();
}

const map<string_view, double> SearchServer::GetWordFrequencies(int document_id) const {
    if (document_to__word_.count(document_id) == 0) {
        return empty_document_;//{pair};
        
    }
    
    return document_to__word_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {

    auto words = GetWordFrequencies(document_id);
    if (words.empty()) {
        return;
    }
    
    for_each(std::execution::par, document_to__word_.at(document_id).begin(),
        document_to__word_.at(document_id).end(), [this, document_id](auto& word) {
            word_to_document_freqs_.at(string(word.first)).erase(document_id);
        });
 
    
    document_to__word_.erase(document_id);
    documents_.erase(document_id);
    indexes_docs.erase(document_id);
}

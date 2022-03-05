#include <vector>
#include <unordered_set>
#include <algorithm>
#include "remove_duplicates.h"
using namespace std;

void RemoveDuplicates(SearchServer &search_server) {
    set<set<string>> unique_docs;
    vector<int> docs_to_delete;
    for (const int id : search_server) {
        const map<string_view, double> word_freqs = search_server.GetWordFrequencies(id);
        set<string> words;
        transform(word_freqs.cbegin(), word_freqs.cend(), inserter(words, words.begin()),
                  [](const pair<string_view, double> &word) {
            return string(word.first);
        });

        if (unique_docs.count(words) == 0) {
            unique_docs.insert(words);
        } else {
            docs_to_delete.push_back(id);
        }
    }

    for (const int id : docs_to_delete) {
        cout << "Found duplicate document id " + to_string(id) << "\n";
        search_server.RemoveDocument(id);
    }
}

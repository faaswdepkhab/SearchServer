#include <string>
#include "string_processing.h"
#include <iostream>


using namespace std;

vector<string_view> SplitIntoWords(string_view str) {
    vector<string_view> result;
    const int64_t pos_end = str.npos;
    while (true) {
        int64_t space = str.find(' ');
        if (space != 0) {
            result.push_back(space == pos_end ? str.substr(0) : str.substr(0, space));
        }
        if (space == pos_end) {
            break;
        } else {
            str.remove_prefix(space+1);
        }
    }
    return result;
}
#include <execution>
#include <algorithm>
#include <iterator>
#include "process_queries.h"

using namespace std;

ComplexVector::BasicIterator::BasicIterator(const std::vector<std::vector<Document>> &c) noexcept {
    data = c.data();
    size = c.size();
    segment = 0;
    offset = 0;
}

ComplexVector::BasicIterator::BasicIterator(const std::vector<std::vector<Document>> &c, const int segment, const int offset) noexcept {
    data = c.data();
    size = c.size();
    this -> segment = segment;
    this -> offset = offset;
}
        
        
[[nodiscard]] bool ComplexVector::BasicIterator::operator==(const BasicIterator& rhs) const noexcept{
    return (data == rhs.data) &&
        (segment == rhs.segment) &&
        (offset== rhs.offset) &&
        (size == rhs.size);
}
            
            
[[nodiscard]] bool ComplexVector::BasicIterator::operator!=(const BasicIterator& rhs) const noexcept {
    return !(*this == rhs);
}
        
[[nodiscard]] Document ComplexVector::BasicIterator::operator*() const noexcept {
    return data[segment][offset];
}
        
        
ComplexVector::BasicIterator& ComplexVector::BasicIterator::operator++() noexcept {
    if (offset < (data[segment].size() - 1)) {
        offset++;
    } else {
        if (segment < (size - 1)) {
            segment++;
            offset = 0;
        }    
        else {
            offset++;
        }
    }
    return *this;
}
            
ComplexVector::BasicIterator ComplexVector::BasicIterator::operator++(int) noexcept {
            auto old_value = *this; // Сохраняем прежнее значение объекта для последующего возврата
            ++(*this); // используем логику префиксной формы инкремента
    return old_value;
}


ComplexVector::ComplexVector(const std::vector<std::vector<Document>> &data) {
    this->data = std::move(data);
}

vector<vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const vector<string>& queries) {
    vector<vector<Document>> res(queries.size());
    transform(execution::par, queries.begin(), queries.end(), res.begin(), [&search_server] (auto &s){ return search_server.FindTopDocuments(s);});
    return res;
}

ComplexVector ProcessQueriesJoined(
    const SearchServer& search_server,
    const vector<string>& queries){
    return ComplexVector(ProcessQueries(search_server, queries));
}
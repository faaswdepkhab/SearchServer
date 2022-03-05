//Вставьте сюда своё решение из урока «‎Очередь запросов».‎
#pragma once

#include <iostream>
#include <vector>

template <typename It>
class IteratorRange{
public:
    IteratorRange(It begin_it, It end_it):
        _begin_it(begin_it), _end_it(end_it){}
    It begin() const {
        return _begin_it;
    }
    It end() const {
        return _end_it;
    }
    size_t size() const {
        return distance(_begin_it, _end_it);
    }
private:
    It _begin_it, _end_it;
};

template <typename It>
std::ostream& operator<<(std::ostream& output, IteratorRange<It> range) {
    auto it = range.begin(); 
    for (int i=0; i < range.size(); i++) {
        output << *it;
        it++;
    }
    
    return output;
}

template <typename It>
class Paginator{
public:
    Paginator(It begin_it, It end_it, int page_size){
        auto pos = begin_it;
        pages = {};
        int curr_size=0;
        auto begin_page=pos;
        while (pos != end_it){
            if (curr_size == page_size){
                curr_size=0;
                pages.push_back(IteratorRange<It>(begin_page, pos));
                begin_page = pos;
            }
            pos++;
            curr_size++;
        }
        pages.push_back(IteratorRange<It>(begin_page, pos));
    }
    
    auto begin() const{
        return pages.begin();
    }
    
    auto end() const{
        return pages.end();
    }
    
    size_t size() const{
        return pages.size();
    }
    private:
        std::vector<IteratorRange<It>> pages;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

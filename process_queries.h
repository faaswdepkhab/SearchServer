#pragma once
#include <vector>
#include <iterator>
#include <list>
#include <string>
#include "search_server.h"
#include "document.h"

class ComplexVector {
private:
    
    class BasicIterator {
        // Класс списка объявляется дружественным, чтобы из методов списка
        // был доступ к приватной области итератора
        friend class ComplexVector;
        
    public:
        // Категория итератора - forward iterator
        using iterator_category = std::forward_iterator_tag;
        
        // Тип элементов, по которым перемещается итератор
        using value_type = Document;
        
        // Тип, используемый для хранения смещения между итераторами
        using difference_type = std::ptrdiff_t;
        
        // Тип указателя на итерируемое значение
        using pointer = value_type*;
        
        // Тип ссылки на итерируемое значение
        using reference = value_type&;
        
        BasicIterator() = default;
        
        BasicIterator(const std::vector<std::vector<Document>> &c) noexcept;
        
        BasicIterator(const std::vector<std::vector<Document>> &c, const int segment, const int offset) noexcept;
        
        
        [[nodiscard]] bool operator==(const BasicIterator& rhs) const noexcept;
            
            
        [[nodiscard]] bool operator!=(const BasicIterator& rhs) const noexcept;
        
        [[nodiscard]] Document operator*() const noexcept;
        
        
        BasicIterator& operator++() noexcept;
            
        BasicIterator operator++(int) noexcept;
        
    private:
        int segment = 0;
        int offset = 0;
        int size = 0;
        const std::vector<Document> *data;
    };

public:
    using value_type = Document;
    using reference = value_type&;
    using const_reference = const value_type&;

    // Итератор, допускающий изменение элементов списка
    using Iterator = BasicIterator;
    // Константный итератор, предоставляющий доступ для чтения к элементам списка
    
    ComplexVector(const std::vector<std::vector<Document>> &data);
    
    [[nodiscard]] Iterator begin() noexcept {
        return Iterator(data);
    }
    
    [[nodiscard]] Iterator end() noexcept {
        return Iterator(data, data.size() - 1, data[data.size() - 1].size() );
    }
    
    
private:
    std::vector<std::vector<Document>> data;
	
};


std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);

ComplexVector ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries); 
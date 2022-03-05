//Вставьте сюда своё решение из урока «‎Очередь запросов».‎
#pragma once

#include <iostream>

struct Document {
    int id;
    double relevance;
    int rating;
    Document(): id(0), relevance(0), rating(0){}
    Document(int id_, double relevance_, int rating_):
        id(id_), relevance(relevance_), rating(rating_){}
};

std::ostream& operator<<(std::ostream& output, Document doc);

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};
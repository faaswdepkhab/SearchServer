//Вставьте сюда своё решение из урока «‎Очередь запросов».‎
#include "document.h"

using namespace std;

ostream& operator<<(ostream& output, Document doc) {
    output << "{ ";
    output << "document_id = " << doc.id;
    output << ", relevance = " << doc.relevance;
    output << ", rating = " << doc.rating;
    output << " }";
    return output;
}
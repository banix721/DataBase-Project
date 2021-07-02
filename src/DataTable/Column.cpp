#include "Column.h"


Column::Column(string name, const type_index type)
{
    if (TypeChecker::WrongType(type)) throw std::invalid_argument("Wrong column type");

    Type = type;
    Name = name;
}
#include "Cell.h"

Cell::Cell(const std::type_info& type, RowState* parentRowState)
{
    if (TypeChecker::WrongType(type)) throw std::invalid_argument("Wrong cell type");

    ParentRowState = parentRowState;
    Types::InitValue(type, value);
}

const char* Cell::Type()
{
    return value.type().name();
}

const std::type_info& Cell::TypeInfo()
{
    return value.type();
}

string Cell::GetString()
{
    stringstream ss;
    if (this->TypeInfo() == typeid(int))
        ss << this->Get<int>();
    else if (this->TypeInfo() == typeid(string))
        ss << this->Get<string>();
    else if (this->TypeInfo() == typeid(double))
        ss << this->Get<double>();
    else if (this->TypeInfo() == typeid(float))
        ss << this->Get<float>();
    else if (this->TypeInfo() == typeid(char))
        ss << this->Get<char>();
    else if (this->TypeInfo() == typeid(uint32_t))
        ss << this->Get<uint32_t>();

    return ss.str();
}

void Cell::CopyFrom(const Cell& cell)
{
    if (value.type() == cell.value.type())
        value = cell.value;
}
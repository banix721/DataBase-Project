#pragma once

#include "Types.h"
#include <sstream>

class Cell
{
public:
    Cell() 
    { 
        ParentRowState = NULL;
    }
    Cell(const std::type_info& type, RowState* parentRowState = NULL);
    Cell(const Cell& cell, RowState* parentRowState = NULL)
    {
        *this = cell;
        this->ParentRowState = parentRowState;
    }

    string GetString();

    template <class T> T Get()
    {
        if (value.type() != typeid(T)) throw;
        return get<T>(value);
    }

    template <class T> void Set(T newValue)
    {
        if (value.type() != typeid(T))  throw;

        if (get<T>(value) != newValue)
        {
            if (ParentRowState != NULL)
            {
                if (*ParentRowState != RowState::Inserted)
                    *ParentRowState = RowState::Changed;
            }
            value = newValue;
        }
    }

    template <class T>  Cell& operator = (T _val)
    {
        Set<T>(_val); 
        return(*this); 
    }

    void CopyFrom(const Cell& cell);
    const char* Type();
    const std::type_info& TypeInfo();

private:
    CellTypeValue value;
    RowState* ParentRowState;
};

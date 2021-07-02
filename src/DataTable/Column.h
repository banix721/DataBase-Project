#pragma once

#include "Types.h"

class Column
{
public:
    Column() {};
    Column(string name, const type_index type);

    type_index Type;
    string Name;
};

class ColumnsCollection
{
public:
    typedef typename map<string, Column>::iterator iterator;
    typedef typename map<string, Column>::const_iterator const_iterator;

    inline iterator begin() { return collection.begin(); }
    inline const_iterator cbegin() const { return collection.cbegin(); }
    inline iterator end() { return collection.end(); }
    inline const_iterator cend() const { return collection.cend(); }

    Column& operator[](const string column)
    {
        return collection[column];
    }

    Column& operator[](const int index)
    {
        ColumnsCollection::iterator it = collection.begin();
        std::advance(it, index);
        return it->second;
    }

    bool Contain(string column)
    {
        return collection.find(column) != collection.end();
    }

    bool Contain(string column) const
    {
        return collection.find(column) != collection.cend();
    }

    ColumnsCollection::iterator find(const string column)
    {
        return collection.find(column);
    }

    ColumnsCollection::const_iterator find(const string column) const
    {
        return collection.find(column);
    }

    size_t size()
    {
        return collection.size();
    }

    size_t size() const
    {
        return size();
    }

    bool empty()
    {
        return collection.empty();
    }

    bool empty() const
    {
        return empty();
    }

private:
    map<string, Column> collection;
};
#pragma once

#include "Cell.h"
#include "Column.h"

class Row
{
public:
    Row() 
    { 
        Id = 0;
        State = None;
    }
    Row(ColumnsCollection* columns);
    Row(const Row& row);

    const ColumnsCollection* Columns;
    uint32_t Id;
    RowState State;

    Cell& operator[](const string column)
    {
        if (Columns->Contain(column) == false) throw "Can't find column: " + column;

        return cells[column];
    }

    size_t size()
    {
        return cells.size();
    }

    bool empty()
    {
        return cells.empty();
    }

    void SetCells(Row& row);

private:
    map<string, Cell> cells;
};

typedef vector<Row> RowsCollection;
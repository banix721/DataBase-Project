#pragma once

#include "Row.h"
#include <vector>
#include <functional>
#include <iomanip>

#define MaxSizeDeleteRows 1000

class DataTable
{
public:
    DataTable() 
    { 
        Name = "";
        MaxId = 0;
        MaxSizeDeletedRowsContainer = MaxSizeDeleteRows;
    }
    DataTable(string name)
    {
        Name = name;
        MaxId = 0;
        MaxSizeDeletedRowsContainer = MaxSizeDeleteRows;
    }

    bool AddColumn(Column column);
    Row& AddRow();
    void ClearRows();
    DataTable Clone();
    void AddRows(const RowsCollection& rows);
    void SetRows(RowsCollection& rows);
    void RemoveRow(const Row& row);
    void SetMaxSizeDeletedRowsContainer(int maxSize);
    void Print();

    ColumnsCollection Columns;
    RowsCollection Rows;
    string Name;
    vector<uint32_t> DeletedRows;

private:
    uint32_t MaxId;
    unsigned int MaxSizeDeletedRowsContainer;
};
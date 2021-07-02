#include "DataTable.h"

bool DataTable::AddColumn(Column column)
{
    if (column.Name.empty()) return false;
    if (Columns.find(column.Name) != Columns.end()) return false;
    
    Columns[column.Name] = column;

    for (RowsCollection::iterator rowIt = Rows.begin(); rowIt != Rows.end(); rowIt++)
    {
        (*rowIt)[column.Name] = Cell(column.Type.type_info());
    }

    return true;
}

Row& DataTable::AddRow()
{
    Row row = Row(&Columns);
    row.State = RowState::Inserted;
    row.Id = ++MaxId;
    Rows.push_back(row);

    return Rows.back();
}

//Copy without cells and rows
DataTable DataTable::Clone()
{
    DataTable dt;

    dt.Name = this->Name;
    dt.Columns = ColumnsCollection(this->Columns);

    return dt;
}

void DataTable::ClearRows()
{
    Rows.clear();
    MaxId = 0;
}

void DataTable::AddRows(const RowsCollection& rows)
{
    for (RowsCollection::const_iterator row = rows.begin(); row != rows.end(); row++)
    {
        Rows.push_back(*row);
        Rows.back().State = RowState::Inserted;
        Rows.back().Id = ++MaxId;
    }
}

void DataTable::SetRows(RowsCollection& rows)
{
    this->ClearRows();
    uint32_t maxId = 0;
    for (RowsCollection::iterator row = rows.begin(); row != rows.end(); row++)
    {
        Row& newRow = this->AddRow();
        newRow.SetCells(*row);
        newRow.Id = row->Id;

        if (newRow.Id > maxId) maxId = newRow.Id;
    }
    MaxId = maxId;
}

void DataTable::RemoveRow(const Row& row)
{
    for (int i = Rows.size() - 1; i >= 0; i--)
    {
        if (&Rows[i] == &row)
        {
            if (DeletedRows.size() > MaxSizeDeletedRowsContainer)
                DeletedRows.erase(DeletedRows.begin());
            DeletedRows.push_back(Rows[i].Id);

            Rows.erase(Rows.begin() + i);
            break;
        }
    }
}

void DataTable::SetMaxSizeDeletedRowsContainer(int maxSize)
{
    MaxSizeDeletedRowsContainer = maxSize;
    int diff = DeletedRows.size() - MaxSizeDeletedRowsContainer;
    if (diff > 0)
    {
        DeletedRows.erase(DeletedRows.begin(), DeletedRows.begin() + diff - 1);
    }
}

void DataTable::Print()
{
    //Print columns
    for (ColumnsCollection::iterator column = this->Columns.begin(); column != this->Columns.end(); column++)
    {
        string type = "string";
        if (column->second.Type != type_index(typeid(string)))
            type = column->second.Type.name();
        string columnDescr = column->first + string(" (") + type + string(")");
        cout << setw(20) << left << columnDescr << right << " |";
    }
    cout << endl;

    //Print rows
    for (RowsCollection::iterator row = Rows.begin(); row != Rows.end(); row++)
    {
        for (ColumnsCollection::const_iterator col = row->Columns->cbegin(); col != row->Columns->cend(); ++col)
        {
            cout << setw(20) << left << (*row)[col->second.Name].GetString() << right << " |";
        }
        cout << endl;
    }
}
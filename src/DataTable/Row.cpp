#include "Row.h"

Row::Row(ColumnsCollection* columns)
{
    Columns = columns;
    Id = 0;
    State = None;

    for (ColumnsCollection::const_iterator colIt = Columns->cbegin(); colIt != Columns->cend(); colIt++)
    {
        string columnName = colIt->first;
        cells[columnName] = Cell(colIt->second.Type.type_info(), &State);
    }
}

Row::Row(const Row& row)
{
    State = row.State;
    this->Columns = row.Columns;
    for (map<string, Cell>::const_iterator cell = row.cells.cbegin(); cell != row.cells.cend(); cell++)
    {
        this->cells[cell->first] = Cell(cell->second, &State);
    }
    this->Id = row.Id;
}

void Row::SetCells(Row& row)
{
    for (ColumnsCollection::const_iterator col = Columns->cbegin(); col != Columns->cend(); ++col)
    {
        if (row.Columns->Contain(col->first) == false) continue;
        if (cells[col->first].TypeInfo() != row[col->first].TypeInfo()) continue;

        cells[col->first].CopyFrom(row[col->first]);
    }
}
#include "DBSqlite.h"

extern "C"
{
#include <stdio.h>
}

bool DBSqlite::Open(string& error)
{
    int rc;

    if (DataBaseName.empty())
    {
        error = "Empty database name";
        return false;
    }

    if (DataBase != NULL)
        Close(); //Active connection to database. Close connection.

    rc = sqlite3_open((Path + DataBaseName).c_str(), &DataBase);
    if (rc)
    {
        error = sqlite3_errmsg(DataBase);
        return false;
    }

    return true;
}

void DBSqlite::Close()
{
    if (DataBase == NULL) return;

    sqlite3_close(DataBase);
    DataBase = NULL;
}

bool DBSqlite::BeginTransaction()
{
    char* zErrMsg = 0;
    int rc = 0;

    if (DataBase == NULL) return false;

    rc = sqlite3_exec(DataBase, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

bool DBSqlite::EndTransaction()
{
    char* zErrMsg = 0;
    int rc = 0;

    if (DataBase == NULL) return false;

    rc = sqlite3_exec(DataBase, "END TRANSACTION", NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

int DBSqlite::TableExist(string tableName, string& error)
{
    sqlite3_stmt* stmt = NULL;
    char sqlQuery[512];
    int rowCount = -1;

    if (Open(error) == false)
    {
        return -1;
    }

    //SprawdŸ czy tabela istnieje
    sprintf(sqlQuery, "select count(type) from sqlite_master where type = 'table' and name = '%s';", tableName.c_str());
    int rc = sqlite3_prepare_v2(DataBase, sqlQuery, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        error = "error handling -> statement not prepared\n";
        // error handling -> statement not prepared
        goto end;
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) 
    {
        error = "error handling -> no rows returned, or an error occurred\n";
        // error handling -> no rows returned, or an error occurred
        goto end;
    }
    rowCount = sqlite3_column_int(stmt, 0);

    end:

    sqlite3_finalize(stmt);
    Close();

    return rowCount;
}

bool DBSqlite::CreateTable(DataTable dataTable, string& error)
{
    char* zErrMsg = 0;
    int rc = 0;
    string command = "";
    ColumnsCollection::iterator lastColumn;
    bool ret = true;

    if (dataTable.Name.empty())
    {
        error = "Data table has no name";
        return false;
    }
    if (dataTable.Columns.empty())
    {
        error = "Data table has no columns";
        return false;
    }

    if (Open(error) == false)
    {
        return false;
    }

    if (BeginTransaction() == false)
    {
        error = "BeginTransaction error";
        ret = false;
        goto end;
    }

    command = string("CREATE TABLE ") + dataTable.Name + string(" (");

    //Create table
    //Get last column
    lastColumn = dataTable.Columns.end();
    --lastColumn;

    for (ColumnsCollection::iterator colIt = dataTable.Columns.begin(); colIt != dataTable.Columns.end(); colIt++)
    {
        Column column = colIt->second;
        if (column.Name.empty())
        {
            error = "Column has no name";
            ret = false;
            goto end;
        }

        string type = Types::ToSqliteType(column.Type);
        if (type.empty())
        {
            error = "Column has wrong type";
            ret = false;
            goto end;
        }

        command += column.Name + string(" ") + type;
        if (colIt != lastColumn)
            command += string(", ");
    }

    command += string(");");

    rc = sqlite3_exec(DataBase, command.c_str(), NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        error = zErrMsg;
        sqlite3_free(zErrMsg);
        ret = false;
        goto end;
    }

    if (InsertRows(dataTable.Rows, dataTable.Name, false, false, error) == false)
    {
        ret = false;
        goto end;
    }

    end:

    EndTransaction();
    Close();

    return ret;
}

bool DBSqlite::GetTable(DataTable& dataTable, string& error)
{
    bool ret = true;
    ColumnsCollection columns;

    if (dataTable.Name.empty())
    {
        error = "Data table has no name";
        return false;
    }

    if (Open(error) == false)
    {
        return false;
    }

    if (GetColumns(dataTable.Name, columns, false, error) == false)
    {
        ret = false;
        goto end;
    }

    dataTable.Columns = columns;

    if (GetCells(dataTable, false, error) == false)
    {
        ret = false;
        goto end;
    }

    for (RowsCollection::iterator row = dataTable.Rows.begin(); row != dataTable.Rows.end(); row++)
    {
        row->State = RowState::None;
    }

end:

    Close();

    return ret;
}

bool DBSqlite::GetColumns(string tableName, ColumnsCollection& columns, bool createConnection, string& error)
{
    char* zErrMsg = NULL;
    bool ret = true;

    if (tableName.empty())
    {
        error = "Data table has no name";
        return false;
    }

    if (createConnection)
    {
        if (Open(error) == false)
            return false;
    }

    //Get columns
    string command = string("PRAGMA table_info(") + tableName + string(");");
    int rc = sqlite3_exec(DataBase, command.c_str(), GetColumnsCallback, &columns, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        error = zErrMsg;
        sqlite3_free(zErrMsg);
        ret = false;
        goto end;
    }

end:

    if (createConnection)
        Close();

    return ret;
}

bool DBSqlite::GetCells(DataTable& dataTable, bool createConnection, string& error)
{
    char* zErrMsg = NULL;
    bool ret = true;

    if (dataTable.Name.empty())
    {
        error = "Data table has no name";
        return false;
    }

    if (dataTable.Columns.empty())
    {
        error = "Data table has no columns";
        return false;
    }

    if (createConnection)
    {
        if (Open(error) == false)
            return false;
    }

    //Get columns
    string command = string("SELECT ROWID,");
    ColumnsCollection::iterator lastColumn = dataTable.Columns.end();
    lastColumn--;
    for (ColumnsCollection::iterator column = dataTable.Columns.begin(); column != dataTable.Columns.end(); column++)
    {
        command += column->first;
        if (lastColumn != column)
            command += string(",");
    }
    command += string(" from ") + dataTable.Name + string("; ");

    DataTable dt = dataTable.Clone();
    int rc = sqlite3_exec(DataBase, command.c_str(), GetCellsCallback, &dt, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        error = zErrMsg;
        sqlite3_free(zErrMsg);
        ret = false;
        goto end;
    }

    dataTable.SetRows(dt.Rows);

end:

    if (createConnection)
        Close();

    return ret;
}

int DBSqlite::GetColumnsCallback(void* readed, int argc, char** argv, char** azColName)
{
    ColumnsCollection* columns;
    columns = (ColumnsCollection*)readed;

    if (argc == 6)
    {
        char name[250];
        sscanf(argv[1], "%249s", name);
        char typeStr[250];
        sscanf(argv[2], "%249s", typeStr);
        boost::typeindex::type_index type;
        Types::SqliteTypeToTypeIndex(typeStr, type);

        (*columns)[name] = Column(name, type);
    }

    return 0;
}

int DBSqlite::GetCellsCallback(void* readed, int argc, char** argv, char** azColName)
{
    DataTable* dataTable;
    dataTable = (DataTable*)readed;
    int index = 0;

    if (dataTable->Columns.size()+1 != (unsigned int)argc) return -1; //+1 because of rowId

    Row& row = dataTable->AddRow();

    //Read rowId column
    unsigned long idRead = 0;
    sscanf(argv[index], "%lu", &idRead);
    row.Id = idRead;
    index++;

    for (ColumnsCollection::iterator column = dataTable->Columns.begin(); column != dataTable->Columns.end(); column++)
    {
        Column col = column->second;
        if (TypeChecker::WrongType(col.Type) == true) continue;
        if (row.Columns->find(col.Name) == row.Columns->cend()) continue; //key doesn't exist in map
        if (argv[index] == NULL) continue;
        Cell& cell = row[col.Name];

        if (col.Type.type_info() == typeid(int))
        {
            int value = 0;
            sscanf(argv[index], "%d", &value);
            cell.Set<int>(value);
        }
        else if (col.Type.type_info() == typeid(string))
        {
            char value[250];
            memset(value, 0, sizeof(value));
            sscanf(argv[index], "%249c", value);
            cell.Set<string>(value);
        }
        else if (col.Type.type_info() == typeid(float))
        {
            float value = 0.0f;
            sscanf(argv[index], "%f", &value);
            cell.Set<float>(value);
        }
        else if (col.Type.type_info() == typeid(double))
        {
            double value = 0.0;
            sscanf(argv[index], "%lf", &value);
            cell.Set<double>(value);
        }
        else if (col.Type.type_info() == typeid(char))
        {
            char value = 0;
            sscanf(argv[index], "%c", &value);
            cell.Set<char>(value);
        }
        else if (col.Type.type_info() == typeid(uint32_t))
        {
            unsigned long value = 0;
            sscanf(argv[index], "%lu", &value);
            cell.Set<uint32_t>(value);
        }

        index++;
    }

    return 0;
}

bool DBSqlite::BindColumns(int columnIndex, Cell& cell, sqlite3_stmt* pStmt, string& error)
{
    int rc = 0;

    if (cell.TypeInfo() == typeid(int))
    {
        rc = sqlite3_bind_int(pStmt, columnIndex, cell.Get<int>());
        if (rc != SQLITE_OK) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_bind_int 1\n";
            return false;
        }
    }
    else if (cell.TypeInfo() == typeid(string))
    {
        string valueStr = cell.Get<string>();
        rc = sqlite3_bind_text(pStmt, columnIndex, valueStr.c_str(), valueStr.size(), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_bind_text 1\n";
            return false;
        }
    }
    else if (cell.TypeInfo() == typeid(float))
    {
        rc = sqlite3_bind_double(pStmt, columnIndex, cell.Get<float>());
        if (rc != SQLITE_OK) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_bind_double (flaot) 1\n";
            return false;
        }
    }
    else if (cell.TypeInfo() == typeid(double))
    {
        rc = sqlite3_bind_double(pStmt, columnIndex, cell.Get<double>());
        if (rc != SQLITE_OK) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_bind_double 1\n";
            return false;
        }
    }
    else if (cell.TypeInfo() == typeid(uint32_t))
    {
        rc = sqlite3_bind_int64(pStmt, columnIndex, (sqlite_int64)cell.Get<uint32_t>());
        if (rc != SQLITE_OK) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_bind_int64\n";
            return false;
        }
    }

    return true;
}

bool DBSqlite::InsertRows(RowsCollection& rows, string tableName, bool createConnection, bool createTransaction, string& error)
{
    bool ret = true;
    string command = "";
    ColumnsCollection::const_iterator lastColumn;
    stringstream ss;
    sqlite3_stmt* pStmt = NULL;
    int rc = 0;

    if (tableName.empty())
    {
        error = "tableName empty";
        return false;
    }
    if (rows.empty())
    {
        error = "rows empty";
        return false;
    }
    if (rows[0].empty())
    {
        error = "Cells empty";
        return false;
    }

    if (createConnection)
    {
        if (Open(error) == false)
            return false;
    }
    else
    {
        if (DataBase == NULL)
        {
            error = "Connection wasn't opened";
            ret = false;
            goto end;
        }
    }

    if (createTransaction)
    {
        if (BeginTransaction() == false)
        {
            error = "BeginTransaction error";
            ret = false;
            goto end;
        }
    }

    command = string("INSERT INTO ") + tableName + string(" (ROWID,");

    //set columns
    lastColumn = rows[0].Columns->cend();
    lastColumn--;
    for (ColumnsCollection::const_iterator col = rows[0].Columns->cbegin(); col != rows[0].Columns->cend(); ++col)
    {
        command += col->first;
        if (lastColumn != col)
            command += string(",");
    }
    command += string(") VALUES (");
    for (unsigned int i = 0; i < rows[0].size() + 1; i++) //+1 because rowid
    {
        ss << string("?") << (i + 1);
        if (i != rows[0].size())
            ss << string(",");
    }
    command += ss.str() + string(");");


    rc = sqlite3_prepare_v2(DataBase, command.c_str(), command.size() + 1, &pStmt, NULL);
    if (rc != SQLITE_OK)
    {
        error = "SQL error Insert: sqlite3_prepare_v2\n";
        goto end;
    }

    for (RowsCollection::iterator row = rows.begin(); row != rows.end(); row++)
    {
        rc = sqlite3_bind_int64(pStmt, 1, (sqlite_int64)row->Id);
        if (rc != SQLITE_OK) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_bind_int 1\n";
            ret = false;
            goto end;
        }
        int columnIndex = 2;
        for (ColumnsCollection::const_iterator col = row->Columns->cbegin(); col != row->Columns->cend(); ++col)
        {
            Cell& c = (*row)[col->second.Name];

            if (BindColumns(columnIndex, c, pStmt, error) == false)
            {
                ret = false;
                goto end;
            }
            columnIndex++;
        }
        if (sqlite3_step(pStmt) != SQLITE_DONE) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_step\n";
            const char* err = sqlite3_errmsg(DataBase);
            if(err != NULL)
                error += string(err);
            ret = false;
            goto end;
        }
        sqlite3_reset(pStmt);
        sqlite3_clear_bindings(pStmt);
    }

    //All ok
    for (RowsCollection::iterator row = rows.begin(); row != rows.end(); row++)
        row->State = RowState::None;

end:
    if (pStmt != NULL)
    {
        sqlite3_reset(pStmt);
        sqlite3_clear_bindings(pStmt);
        sqlite3_finalize(pStmt);
    }

    if(createTransaction)
        EndTransaction();
    if (createConnection)
        Close();

    return ret;
}

bool DBSqlite::UpdateRows(RowsCollection& rows, string tableName, bool createConnection, bool createTransaction, string& error)
{
    bool ret = true;
    string command = "";
    ColumnsCollection::const_iterator lastColumn;
    stringstream ss;
    sqlite3_stmt* pStmt = NULL;
    int rc = 0;
    int index = 1;

    if (tableName.empty())
    {
        error = "tableName empty";
        return false;
    }
    if (rows.empty())
    {
        error = "rows empty";
        return false;
    }
    if (rows[0].empty())
    {
        error = "Cells empty";
        return false;
    }

    if (createConnection)
    {
        if (Open(error) == false)
            return false;
    }
    else
    {
        if (DataBase == NULL)
        {
            error = "Connection wasn't opened";
            ret = false;
            goto end;
        }
    }

    if (createTransaction)
    {
        if (BeginTransaction() == false)
        {
            error = "BeginTransaction error";
            ret = false;
            goto end;
        }
    }

    command = string("UPDATE ") + tableName + string(" SET ");

    //set columns
    lastColumn = rows[0].Columns->cend();
    lastColumn--;

    for (ColumnsCollection::const_iterator col = rows[0].Columns->cbegin(); col != rows[0].Columns->cend(); ++col)
    {
        ss << col->first << "=";
        ss << string("?") << index;
        if (lastColumn != col)
            ss << string(",");
        index++;
    }
    ss << " WHERE ROWID == ?" << index << ";";
    command += ss.str();

    rc = sqlite3_prepare_v2(DataBase, command.c_str(), command.size() + 1, &pStmt, NULL);
    if (rc != SQLITE_OK)
    {
        error = "SQL error Insert: sqlite3_prepare_v2\n";
        const char* err = sqlite3_errmsg(DataBase);
        if (err != NULL)
            error += string(err);
        ret = false;
        goto end;
    }

    for (RowsCollection::iterator row = rows.begin(); row != rows.end(); row++)
    {
        int columnIndex = 1;
        for (ColumnsCollection::const_iterator col = row->Columns->cbegin(); col != row->Columns->cend(); ++col)
        {
            Cell& c = (*row)[col->second.Name];

            if (BindColumns(columnIndex, c, pStmt, error) == false)
            {
                ret = false;
                goto end;
            }
            columnIndex++;
        }
        rc = sqlite3_bind_int64(pStmt, columnIndex, (sqlite_int64)row->Id);
        if (rc != SQLITE_OK) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_bind_int 1\n";
            ret = false;
            goto end;
        }
        if (sqlite3_step(pStmt) != SQLITE_DONE) {
            error = "AMS: SQL error InsertDBaseIMP: sqlite3_step\n";
            ret = false;
            goto end;
        }
        sqlite3_reset(pStmt);
        sqlite3_clear_bindings(pStmt);
    }

    //All ok
    for (RowsCollection::iterator row = rows.begin(); row != rows.end(); row++)
        row->State = RowState::None;

end:
    if (pStmt != NULL)
    {
        sqlite3_reset(pStmt);
        sqlite3_clear_bindings(pStmt);
        sqlite3_finalize(pStmt);
    }

    if (createTransaction)
        EndTransaction();
    if (createConnection)
        Close();

    return ret;
}

bool DBSqlite::DeleteRows(vector<uint32_t> IDcollection, string tableName, bool createConnection, bool createTransaction, string& error)
{
    char* zErrMsg = NULL;
    bool ret = true;
    string command = "";
    vector<uint32_t>::iterator lastId;
    stringstream ss;
    int rc = 0;

    if (tableName.empty())
    {
        error = "tableName empty";
        return false;
    }
    if (IDcollection.empty())
    {
        error = "rows empty";
        return false;
    }

    if (createConnection)
    {
        if (Open(error) == false)
            return false;
    }
    else
    {
        if (DataBase == NULL)
        {
            error = "Connection wasn't opened";
            ret = false;
            goto end;
        }
    }

    if (createTransaction)
    {
        if (BeginTransaction() == false)
        {
            error = "BeginTransaction error";
            ret = false;
            goto end;
        }
    }

    command = string("DELETE FROM ") + tableName + string(" WHERE ROWID IN (");

    lastId = IDcollection.end();
    lastId--;
    for (vector<uint32_t>::iterator id = IDcollection.begin(); id != IDcollection.end(); id++)
    {
        ss << *id;
        if (lastId != id)
            ss << string(",");
    }
    ss << ");";
    command += ss.str();

    /* Execute SQL statement */
    rc = sqlite3_exec(DataBase, command.c_str(), NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) 
    {
        error = "AMS: SQL error DeleteLastRows: %s\n";
        sqlite3_free(zErrMsg);
        ret = false;
        goto end;
    }

end:

    if (createTransaction)
        EndTransaction();
    if (createConnection)
        Close();

    return ret;
}

bool DBSqlite::UpdateChanges(DataTable& dataTable, bool createConnection, bool createTransaction, string& error)
{
    bool ret = true;
    string command = "Select rowid,";
    RowsCollection rowsToInsert;
    RowsCollection rowsToUpdate;

    if (dataTable.Name.empty())
    {
        error = "Data table has no name";
        return false;
    }

    if (createConnection)
    {
        if (Open(error) == false)
            return false;
    }
    else
    {
        if (DataBase == NULL)
        {
            error = "Connection wasn't openned";
            ret = false;
            goto end;
        }
    }
    if (createTransaction)
    {
        if (BeginTransaction() == false)
        {
            error = "BeginTransaction error";
            ret = false;
            goto end;
        }
    }

    for (RowsCollection::iterator row = dataTable.Rows.begin(); row != dataTable.Rows.end(); row++)
    {
        if (row->State == RowState::Inserted)
            rowsToInsert.push_back(*row);
        if (row->State == RowState::Changed)
            rowsToUpdate.push_back(*row);
    }
    if (dataTable.DeletedRows.empty() == false)
    {
        if (DeleteRows(dataTable.DeletedRows, dataTable.Name, false, false, error) == false)
            return false;
        dataTable.DeletedRows.clear();
    }
    if (rowsToUpdate.empty() == false)
    {
        if (UpdateRows(rowsToUpdate, dataTable.Name, false, false, error) == false)
            return false;
    }
    if (rowsToInsert.empty() == false)
    {
        if (InsertRows(rowsToInsert, dataTable.Name, false, false, error) == false)
            return false;
    }

    //All ok.
    for (RowsCollection::iterator row = dataTable.Rows.begin(); row != dataTable.Rows.end(); row++)
    {
        row->State = RowState::None;
    }

end:

    if (createTransaction)
        EndTransaction();
    if (createConnection)
        Close();

    return ret;
}

bool DBSqlite::UpdateChanges(DataTable& dataTable, string& error)
{
    return UpdateChanges(dataTable, true, true, error);
}
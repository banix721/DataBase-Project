#pragma once

#include "../DataTable/DataTable.h"

extern "C"
{
    #include "sqlite3.h"
}

using namespace std;

class DBSqlite
{
public:
    DBSqlite(string path, string dataBaseName)
    {
        Path = path;
        DataBaseName = dataBaseName;
        DataBase = NULL;
    }

    bool Open(string& error);
    void Close();
    bool BeginTransaction();
    bool EndTransaction();
    int TableExist(string tableName, string& error);
    bool CreateTable(DataTable dataTable, string& error);
    bool GetTable(DataTable& dataTable, string& error);
    bool GetColumns(string tableName, ColumnsCollection& columns, bool createConnection, string& error);
    bool GetCells(DataTable& dataTable, bool createConnection, string& error);
    bool InsertRows(RowsCollection& rows, string tableName, bool createConnection, bool createTransaction, string& error);
    bool UpdateRows(RowsCollection& rows, string tableName, bool createConnection, bool createTransaction, string& error);
    bool DeleteRows(vector<uint32_t> IDcollection, string tableName, bool createConnection, bool createTransaction, string& error);
    bool UpdateChanges(DataTable& dataTable, bool createConnection, bool createTransaction, string& error);
    bool UpdateChanges(DataTable& dataTable, string& error);

    string Path;
    string DataBaseName;

private:
    static int GetColumnsCallback(void* readed, int argc, char** argv, char** azColName);
    static int GetCellsCallback(void* readed, int argc, char** argv, char** azColName);
    bool BindColumns(int columnIndex, Cell& cell, sqlite3_stmt* pStmt, string& error);

    sqlite3* DataBase;
};
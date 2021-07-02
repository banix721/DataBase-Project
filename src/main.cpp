/*------------------------------------------------------------------------------*

SQLITE3_LIB

Dawid Nowakowski, MIKRONIKA 2021

*------------------------------------------------------------------------------*/

#include "DataTable/DataTable.h"
#include "DataBaseSqlite/DBSqlite.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
}

int main(int argc, char** argv)
{
    {
        DataTable dt;
        dt.Name = "TabTest";
        dt.AddColumn(Column("Id", type_id<int>()));
        dt.AddColumn(Column("Name", type_id<string>()));
        dt.AddColumn(Column("Float", type_id<float>()));
        dt.AddColumn(Column("Double", type_id<double>()));

        dt.AddRow();
        dt.AddRow();

        dt.Rows[1]["Id"].Set<int>(721);
        //lub
        dt.Rows[1]["Id"] = 722;
        dt.Rows[1]["Float"].Set<float>(12.56f);
        dt.Rows[1]["Double"].Set<double>(55.88);
        dt.Rows[1]["Name"].Set<string>("Banan");
        //lub
        dt.Rows[1]["Name"] = string("Banan2");

        int w1 = dt.Rows[1]["Id"].Get<int>();
        string w2 = dt.Rows[1]["Name"].Get<string>();
        int w3 = dt.Rows[0]["Id"].Get<int>();
        string w4 = dt.Rows[0]["Name"].Get<string>();

        string error = "";
        DBSqlite db("", "BazaTestowa");
        
        int exist = db.TableExist(dt.Name, error);
        if (exist == 0)
        {
            db.CreateTable(dt, error);
        }
        else if (exist > 0)
        {
            DataTable dt2("TabTest");
            db.GetTable(dt2, error);
            dt2.Print();

            int c1 = dt2.Rows[1]["Id"].Get<int>();
            float c2 = dt2.Rows[1]["Float"].Get<float>();
            double c3 = dt2.Rows[1]["Double"].Get<double>();
            string c4 = dt2.Rows[1]["Name"].Get<string>();

            dt2.Rows[0]["Name"].Set<string>("Nowa2 War2");
            dt2.Rows[1]["Id"].Set<int>(++c1);

            dt2.AddRow();
            dt2.AddRow();
            dt2.AddRow();

            for (int i = 3; i >= 2; i--)
                dt2.RemoveRow(dt2.Rows[i]);

            bool ret = db.UpdateChanges(dt2, error);
            db.GetTable(dt2, error);
            dt2.Print();
        }
    }

	return 0;
}


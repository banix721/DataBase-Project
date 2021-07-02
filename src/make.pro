TEMPLATE    = app
CONFIG      += console

SOURCES     = main.cpp \
			  DataBaseSqlite/sqlite3.c \
              DataBaseSqlite/DBSqlite.cpp \
              DataTable/Cell.cpp \
              DataTable/Column.cpp \
              DataTable/Row.cpp \
              DataTable/DataTable.cpp \

HEADERS     = DataBaseSqlite/sqlite3.h\
			  DataBaseSqlite/sqlite3ext.h\
              DataBaseSqlite/DBSqlite.h \
              DataTable/Cell.h \
              DataTable/Column.h \
              DataTable/Row.h \
              DataTable/DataTable.h \
              DataTable/Types.h \


TARGET      = sqlite3_lib

unix:LIBS += -lstdc++
arm-linux-gcc: LIBS += -liberty

omap-linux-gcc {
    QMAKE_CXXFLAGS += -std=c++0x
    INCLUDEPATH += ../thirdparty/boost-1.72.0
}

qt4 {
    LIBS += -lmms_kfunctions
} else {
    LIBS += -lkfunctions
}
LIBS	+= -lemb_comm


win32:LIBS += -lWs2_32 -lUser32
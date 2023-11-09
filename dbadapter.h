#ifndef DBADAPTER_H
#define DBADAPTER_H

#include <queue>
#include <string>
#include "sqlite/sqlite3.h"

class DBAdapter
{
private:
    sqlite3 *db;
    char *errMsg = nullptr;
public:
    DBAdapter();
    ~DBAdapter();
    void Parse( std::string &, std::queue<std::string>&);
};

#endif // DBADAPTER_H

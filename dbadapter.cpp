#include "dbadapter.h"

const std::string sql_create{"BEGIN;"
                             "CREATE TABLE IF NOT EXISTS A( id INT PRIMARY KEY NOT NULL, name TEXT);" \
                             "CREATE TABLE IF NOT EXISTS B( id INT PRIMARY KEY NOT NULL, name TEXT);"
                             "COMMIT;"};

const std::string intersection_query = "SELECT a.id, a.name, b.name FROM a INNER JOIN b ON a.id=b.id;";

const std::string difference_query = "SELECT a.id, a.name, b.name FROM a LEFT JOIN b ON a.id=b.id " \
                                     "union " \
                                     "SELECT b.id, a.name, b.name FROM b LEFT JOIN a ON a.id=b.id " \
                                     "except " \
                                     "SELECT a.id, a.name, b.name FROM a INNER JOIN b ON a.id=b.id;";

std::vector<std::string> split(const std::string &str, char d)
{
    std::vector<std::string> r;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(d);
    while(stop != std::string::npos)
    {
        r.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop = str.find_first_of(d, start);
    }

    r.push_back(str.substr(start));

    return r;
}

DBAdapter::DBAdapter()
{
    // установить соединение с базой данных
    // открыть/создать
    if(sqlite3_open("join_test.db", &db)){
        db = nullptr;
    }
    else{
        if( sqlite3_exec(db, sql_create.c_str(), nullptr, nullptr, &errMsg)){
            sqlite3_free(errMsg);
        }
    }
}

DBAdapter::~DBAdapter()
{
    if( db ){
        sqlite3_close(db);
    }
}

void DBAdapter::Parse( std::string &msg, std::queue<std::string> &result){
    std::string sql;

    //callback
    static auto getquery = [](void *data, int argc, char **argv, char **azColName){
        std::string str;
        for(auto i = 0; i < argc; ++i){
            if( argv[i] ){
                str += argv[i];
            }
            str += (i != (argc-1)) ? "," : "\n";
        }
        ((std::queue<std::string> *)data)->push(str);
        return 0;
    };

    auto cmd = split(msg, ' ');

    if ( cmd[0] == "INSERT" && cmd.size() == 4){
        sql ="INSERT INTO " + cmd[1] + " VALUES(" + cmd[2] + ", '" + cmd[3] + "');";
    } else if( cmd[0] == "TRUNCATE" && cmd.size() == 2 ){
        sql = "DELETE FROM " + cmd[1] + ";";
    } else if( cmd[0] == "INTERSECTION" ){
        sql = intersection_query;
    } else if( cmd[0] == "SYMMETRIC_DIFFERENCE" ){
        sql =  difference_query;
    }

    if( sql.length()){
        sql = "BEGIN; " + sql;
        if( auto rc = sqlite3_exec(db, sql.c_str(), getquery, (void*)&result, &errMsg)){
            std::string ans = "ERR ";

            if( rc == SQLITE_CONSTRAINT){
                ans += "duplicate " + cmd[2];
            } else{
                ans += errMsg;
            }
            ans += "\n";

            result.push(ans);

            sqlite3_free(errMsg);

            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, &errMsg);

        } else {
            result.push("OK\n");
            sqlite3_exec(db, "COMMIT", nullptr, nullptr, &errMsg);
        }
    } else{
        result.push("ERR wrong command\n");
    }
}

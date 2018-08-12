#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <cstring>
#include <sqlite3.h>
#include <map>
#include "munch.hpp"

using std::string;
using std::vector;

namespace munch{

    void print_usage_and_exit(bool help, string msg="")
    {
        using std::cout;
        using std::endl;
        if(msg != "")
            cout << msg << endl;
        cout << "Usage:" << endl;
        cout << "munch [options] [command] [tags or files]" << endl;
        if(!help) exit(EXIT_FAILURE);
        cout << endl;
        cout << "Commands: " << endl;
        cout << " update [files]\t\t";
        cout << "update the munch database using files" << endl;
        cout << " search [tags]\t\t"
             << "show notes that match all tags" << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << " -d=DB, --database=DB\t"
             << "use DB rather than munch.sqlite3";
        cout << endl;
        cout << " -h, --help\t\t" << "show help and exit" << endl;
        exit(EXIT_FAILURE);
    }

    /* parses command line options (any argments starting with '-') */
    munch_options parse_options(vector<string>& options)
    {
        munch_options mo {};
        for(auto a: options){
            string param {a};
            if(param.find("--database=") == 0)
                if(param.size() >= 12)
                    mo.database = param.substr(11);
                else
                    print_usage_and_exit(false, "Error parsing options");
            else if(param.find("-d=") == 0)
                if(param.size() >= 4)
                    mo.database = param.substr(3);
                else
                    print_usage_and_exit(false, "Error parsing options");
            else
                print_usage_and_exit(false, "Error: unrecognised option");
        }
        return mo;
    }

    /* read a file's contents, tags and id if there is one */
    int parse_file(string f, vector<string>& tags,
                   string& contents)
    {
        std::ifstream file (f);
        if(!file.is_open())
            print_usage_and_exit(false, "failed to open given file");
        string line;
        int id=-1;
        bool first_line = true;
        while(getline(file, line)){
            if(line.at(0) == '#')
                tags.push_back(line.substr(1));
            else if((first_line) && (line.at(0) == '%'))
                id=atoi(line.substr(1).c_str());
            else{
                contents += line;
                contents += "\\n";
            }
            first_line = false;
        }
        file.close();
        return id;
    }

    /* sqlite callback */
    static int callback(void *res, int argc, char** argv, char **col)
    {
        query_results *qr= (query_results*)res;
        for(auto i=0; i<argc; ++i){
            if((strcmp(col[i],"id") == 0)||(strcmp(col[i],"MAX(id)") == 0)){
                if(argv[i] == nullptr)
                    qr->id = 0;
                else
                    qr->id = atoi(argv[i]);
            }
            else if (strcmp(col[i], "note") == 0){
                qr->text = string(argv[i]);
            }
            else if (strcmp(col[i], "ptr") == 0){
                qr->search_results.push_back(atoi(argv[i]));
            }
            else {
                print_usage_and_exit(false, col[i]);
            }
        }
        return 0;
    }

    /* prepare an insert or replace statement for execution
       on a ([id,] text) or ([id,] text, num) table */
    void construct_insert(string& res, string verb,
                          string table, int id,
                          string text_field, int num_field=0)
    {
        res = verb;
        res += " INTO ";
        res += table;
        res += " VALUES(";
        if(id > 0){
            char num[32];
            snprintf(num, 31, "%d", id);
            res += num;
            res += ", \"";
        }
        else {
            res += "\"";
        }
        res += text_field;
        if(num_field != 0){
            res += "\",";
            char num[32];
            snprintf(num, 31, "%d", num_field);
            res += num;
            res += ") ";
        }
        else {
            res += "\")";
        }
    }

    /* if SQL stmt does not execute correctly, the program will end */
    void sqlite3_exec_guard(sqlite3 *db, string stmt,
                            int (*callback) (void*,int,char**,char**),
                            void* res, char** err)
    {
        int rc = sqlite3_exec(db, stmt.c_str(), callback, res, err);
        if(rc != SQLITE_OK){
            std::cout << "sqlite3 error: " << *err << std::endl;
            sqlite3_free(*err);
            sqlite3_close(db);
            print_usage_and_exit(false, "SQL error");
        }
    }

    /* searches the database for a union match of the specified tags */
    vector<int> search(string db_path,
                            vector<string> tags)
    {
        /* check to see if database exists */
        sqlite3 *db;
        char *err=0;
        if(sqlite3_open(db_path.c_str(), &db)){
            sqlite3_close(db);
            print_usage_and_exit(false, "Error opening database");
        }
        const string base_stmt = "SELECT ptr from tags WHERE tag='";
        const string quote = "'";
        std::map<int,int> results {};
        auto n = 0;
        for(auto t: tags){
            string stmt = base_stmt + t + quote;
            query_results qr;
            sqlite3_exec_guard(db, stmt.c_str(), callback, &qr, &err);
            for(auto id: qr.search_results){
                if(results.find(id) == results.end())
                    results[id] = 1;
                else
                    results[id] = results[id] + 1;
            }
            n++;
        }
        vector<int> ids {};
        for(auto& pair: results){
            if(pair.second == n){
                ids.push_back(pair.first);
            }
        }
        sqlite3_close(db);
        return ids;
    }

    /* updates the database with the specified files */
    void update_database(string db_path,
                         vector<string> files)
    {
        /* check to see if database exists */
        sqlite3 *db;
        if(sqlite3_open(db_path.c_str(), &db)){
            sqlite3_close(db);
            print_usage_and_exit(false, "Error opening database");
        }
        if(files.empty()){
            sqlite3_close(db);
            print_usage_and_exit(false, "No files specified");
        }
        for(auto f: files){
            string contents;
            vector<string> tags {};
            auto id = parse_file(f, tags, contents);
            string stmt =
                "CREATE TABLE IF NOT EXISTS "
                "notes(id INTEGER PRIMARY KEY, note TEXT)";
            char *err = 0;
            query_results qr;
            sqlite3_exec_guard(db, stmt.c_str(), callback, nullptr, &err);
            auto note_id = 0;
            if(id <= 0){
                // creating a new note (0= error parsing id with atoi()
                stmt = "SELECT MAX(id) FROM notes";
                sqlite3_exec_guard(db, stmt.c_str(), callback, &qr, &err);
                note_id = qr.id+1;
                construct_insert(stmt, "INSERT", "notes", note_id, contents);
            }
            else {
                //update an existing note
                note_id = id;
                construct_insert(stmt,"REPLACE", "notes", note_id, contents);
            }
            sqlite3_exec_guard(db, stmt.c_str(), callback, nullptr, &err);
            stmt =
                "CREATE TABLE IF NOT EXISTS "
                "tags(tag TEXT, ptr INTEGER)";
            sqlite3_exec_guard(db, stmt.c_str(), callback, nullptr, &err);
            for(auto t: tags){
                construct_insert(stmt, "INSERT", "tags", -1, t, note_id);
                sqlite3_exec_guard(db, stmt.c_str(), callback,nullptr, &err);
            }
        }
        sqlite3_close(db);
    }

    int entry(int argc, char* argv[])
    {
        string command = "";
        vector<string> passed_options {};
        vector<string> args {};
        for(auto i=1; i<argc; ++i){
            string param {argv[i]};
            if((param == "-h") || (param == "--help"))
                print_usage_and_exit(true);
            if(param.at(0) == '-')
                passed_options.push_back(param);
            else
                if(command == "")
                    command = param;
                else if ((command == "search") || (command == "update"))
                    args.push_back(param);
        }
        munch_options options = parse_options(passed_options);
        if(command == "update"){
            update_database(options.database, args);
        }
        else if (command == "search"){
            vector<int> results(search(options.database, args));
            for(auto r: results)
                std::cout << r;
        }
        else{
            std::cout << "Command not recognised. Use " << argv[0];
            std::cout << " --help for more info" << std::endl;
        }
        return 0;
    }

}

int main(int argc, char* argv[])
{
    if(argc < 2){
        munch::print_usage_and_exit(false, "Error: not enough args");
    }
    munch::entry(argc, argv);
}

#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <cstring>
#include <sqlite3.h>
#include "munch.h"

namespace munch{

    void print_usage_and_exit(bool help, std::string msg="")
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
    munch_options parse_options(std::vector<std::string>& options)
    {
        munch_options mo {};
        for(auto a: options){
            std::string param {a};
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
    int parse_file(std::string f, std::vector<std::string>& tags,
                   std::string& contents)
    {
        std::ifstream file (f);
        if(!file.is_open())
            print_usage_and_exit(false, "failed to open given file");
        std::string line;
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
            if((strcmp(col[i], "id") == 0) || (strcmp(col[i], "MAX(id)") == 0)){
                qr->id = atoi(argv[i]);
            }
            else if (strcmp(col[i], "note") == 0){
                qr->note = std::string(argv[i]);
            }
            else {
                print_usage_and_exit(false, col[i]);
            }
        }
        return 0;
    }

    /* updates the database with the specified files */
    void update_database(std::string db_path, std::vector<std::string> files)
    {
        /* check to see if database exists */
        sqlite3 *db;
        auto rc = sqlite3_open(db_path.c_str(), &db);
        if(rc){
            sqlite3_close(db);
            print_usage_and_exit(false, "Error opening database");
        }
        if(files.empty()){
            sqlite3_close(db);
            print_usage_and_exit(false, "No files specified");
        }
        using std::string;
        for(auto f: files){
            string contents;
            std::vector<string> tags {};
            auto id = parse_file(f, tags, contents);
            string stmt =
                "CREATE TABLE IF NOT EXISTS "
                "notes(id INTEGER PRIMARY KEY, note TEXT)";
            char *err = 0;
            query_results qr;
            rc = sqlite3_exec(db, stmt.c_str(), callback, &qr, &err);
            if(rc != SQLITE_OK){
                std::cout << "sqlite3: " << err << std::endl;
                sqlite3_free(err);
                print_usage_and_exit(false, "SQL error");
            }
            if(id <= 0){
                // creating a new note (0= error parsing id with atoi()
                stmt = "SELECT MAX(id) FROM notes";
                rc = sqlite3_exec(db, stmt.c_str(), callback, &qr, &err);
                if(rc != SQLITE_OK){
                    std::cout << "sqlite3: " << err << std::endl;
                    sqlite3_free(err);
                    print_usage_and_exit(false, "SQL error");
                }
                stmt = "INSERT INTO notes VALUES(";
                char new_id[32];
                snprintf(new_id, 31, "%d", qr.id + 1);
                stmt += new_id;
                stmt += ", \"";
                stmt += contents;
                stmt += "\")";
                rc = sqlite3_exec(db, stmt.c_str(), callback, &qr, &err);
                if(rc != SQLITE_OK){
                    std::cout << "sqlite3: " << err << std::endl;
                    sqlite3_free(err);
                    print_usage_and_exit(false, "SQL error");
                }
            }
            else {
                // updating an existing note
            }

        }
        sqlite3_close(db);
    }

    int main(int argc, char* argv[])
    {
        std::string command = "";
        std::vector<std::string> passed_options {};
        std::vector<std::string> args {};
        for(auto i=1; i<argc; ++i){
            std::string param {argv[i]};
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
            //search(db, tags);
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
    munch::main(argc, argv);
}

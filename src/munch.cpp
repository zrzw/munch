#include <iostream>
#include <vector>
#include <stdlib.h>
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
    cout << " search [tags]\t\t" << "show notes that match all tags" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << " -d=DB, --database=DB\t" << "use DB rather than munch.sqlite3";
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

  void add_file(std::string database_path, std::string file_path) { }

  /* updates the munch database with the specified files */
  void update_database(std::string db_path, std::vector<std::string> files)
  {
    /* check to see if database exists */
    sqlite3 *db;
    int rc = sqlite3_open(db_path.c_str(), &db);
    if(rc)
      print_usage_and_exit(false, "Error opening database");
    if(files.empty())
      print_usage_and_exit(false, "No files specified");
    //TODO open then call add_file for each file
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
    munch::print_usage_and_exit(false, "Error: not enough args, exiting");
  }
  munch::main(argc, argv);
}

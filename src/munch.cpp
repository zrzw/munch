#include <iostream>
#include <vector>
#include <stdlib.h>
#include "munch.h"

namespace munch{

  void print_usage_and_exit(bool help, std::string msg="")
  {
    using std::cout;
    using std::endl;
    if(msg != "")
      cout << msg << endl;
    cout << "Usage:" << endl;
    cout << "munch [options] [command]" << endl;
    cout << "munch [options] [command] {tags}" << endl;
    if(!help) exit(EXIT_FAILURE);
    cout << endl;
    cout << "Commands: " << endl;
    cout << " update\t\t\t" << "update the munch database using files in ." << endl;
    cout << " search {tags}\t\t" << "show notes that match all tags" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << " -d=DB, --database=DB\t" << "use DB rather than ~/munch.sqlite";
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
          mo.database = param.substr(4);
        else
          print_usage_and_exit(false, "Error parsing options");
      else
        print_usage_and_exit(false, "Error: unrecognised option");
    }
    return mo;
  }

  int main(int argc, char* argv[])
  {
    std::string command = "";
    std::vector<std::string> passed_options {};
    std::vector<std::string> tags {};
    for(auto i=1; i<argc; ++i){
      std::string param {argv[i]};
      if((param == "-h") || (param == "--help"))
        munch::print_usage_and_exit(true);
      if(param.at(0) == '-')
          passed_options.push_back(param);
      else
        if(command == "")
          command = param;
        else if (command == "search")
          tags.push_back(param);
    }
    munch_options options = parse_options(passed_options);
    if(command == "update"){
      //update(db);
    }
    else if (command == "search"){
      //search(db, tags);
    }
    else{
      std::cout << "Command not recognised. Use " << argv[0];
      std::cout << " --help for more info" << std::endl;
    }
  }

}

int main(int argc, char* argv[])
{
  if(argc < 2){
    munch::print_usage_and_exit(false, "Error: not enough args, exiting");
  }
  munch::main(argc, argv);
}

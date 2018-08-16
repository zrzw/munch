/*
 *	  auxiliary.cpp - system input and output functions
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <sqlite3.h>
#include "munch.hpp"

using namespace munch;

/* parses command line options (any argments starting with '-') */
munch::munch_options munch::parse_options(std::vector<std::string>& options)
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


void munch::print_usage_and_exit(bool help, std::string msg)
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

void munch::print_error_and_exit(std::string msg)
{
	if(msg != "")
		std::cout << "Error: " << msg << std::endl;
	exit(EXIT_FAILURE);
}

void munch::display_note(sqlite3* db, int note_id)
{
	std::cout << "%" << note_id << std::endl;
	std::vector<std::string> tags = get_tags(db, note_id);
	for(auto& t : tags)
		std::cout << "#" << t << std::endl;
	std::string note_string = get_note(db, note_id);
	size_t start=0, current=0;
	for(; current<note_string.size(); ++current){
		if (note_string[current] == 'n')
			if (note_string[current-1] == '\\'){
				std::cout << note_string.substr(start, current-start-1);
				std::cout << std::endl;
				start = current + 1;
				continue;
			}
	}
	if(start < current)
		std::cout << note_string.substr(start, current-start);
}

/* read a file's contents, tags and id if there is one */
int munch::parse_file(std::string f,
					  std::vector<std::string>& tags,
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

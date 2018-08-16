/*
 *    Munch - snippet storing and tagging tool with sqlite backend
 */

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <cstring>
#include <sqlite3.h>
#include "munch.hpp"

using namespace munch;

/* searches the database for a union match of the specified tags */
std::vector<int> munch::search(sqlite3* db, std::vector<std::string> tags)
{
	char *err=0;
	const std::string base_stmt = "SELECT ptr from tags WHERE tag='";
	const std::string quote = "'";
	std::map<int,int> results {};
	auto n = 0;
	for(auto t: tags){
		sanitise(t);
		std::string stmt = base_stmt + t + quote;
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
	std::vector<int> ids {};
	for(auto& pair: results){
		if(pair.second == n){
			ids.push_back(pair.first);
		}
	}
	return ids;
}

/* updates the database with the specified files */
void munch::update_database(sqlite3* db, std::vector<std::string> files)
{
	create_notes_table(db);		// create if not exists
	create_tags_table(db);		// create if not exists
	for(auto f: files){
		std::string contents;
		std::vector<std::string> tags {};
		auto id = parse_file(f, tags, contents);
		std::string verb = (id > 0) ? "REPLACE" : "INSERT";
		id = (id > 0) ? id : get_max_note_id(db) + 1;
		std::string stmt;
		char *err = nullptr;
		construct_insert(stmt, verb, "notes", id, contents);
		sqlite3_exec_guard(db, stmt.c_str(), callback, nullptr, &err);
		for(auto t: tags){
			construct_insert(stmt, "INSERT", "tags", -1, t, id);
			sqlite3_exec_guard(db, stmt.c_str(), callback, nullptr, &err);
		}
	}
}

/* entry point for an instance of the munch program */
int munch::entry(int argc, char* argv[])
{
	std::string command = "";
	std::vector<std::string> passed_options {};
	std::vector<std::string> args {};
	for(auto i=1; i<argc; ++i){
		std::string param {argv[i]};
		if((param == "-h") || (param == "--help"))
			print_usage_and_exit(true, "");
		if(param.at(0) == '-')
			passed_options.push_back(param);
		else
			if(command == "")
				command = param;
			else if ((command == "search") || (command == "update"))
				args.push_back(param);
	}
	munch_options options = parse_options(passed_options);
	sqlite3* db;
	if(sqlite3_open(options.database.c_str(), &db)){
		sqlite3_close(db);
		print_error_and_exit("Couldn't open database " + options.database);
	}
	if(command == "update"){
		if(args.empty())
			print_error_and_exit("No files specified");
		update_database(db, args);
	}
	else if (command == "search"){
		std::vector<int> results(search(db, args));
		for(auto r: results)
			display_note(db, r);
	}
	else{
		std::cout << "Command not recognised. Use " << argv[0];
		std::cout << " --help for more info" << std::endl;
	}
	sqlite3_close(db);
	return 0;
}

int main(int argc, char* argv[])
{
	//test();
	if(argc < 2){
		munch::print_error_and_exit("not enough args");
	}
	munch::entry(argc, argv);
}

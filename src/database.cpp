/*
 *    database.cpp - sqlite3 database access functions
 */
#include <iostream>
#include <sqlite3.h>
#include <cstring>
#include "munch.hpp"

using namespace munch;

/* replace instances of " with ' in query text fields */
void munch::sanitise(std::string& stmt)
{
	size_t pos = 0;
	const std::string q = "'";
	while (pos < stmt.length()){
		size_t f_pos = stmt.find("\"", pos);
		if(f_pos == std::string::npos)
			break;
		stmt.replace(f_pos, 1, q);
		pos = f_pos + 1;
	}
}

void munch::create_notes_table(sqlite3 *db)
{
	std::string stmt =
			"CREATE TABLE IF NOT EXISTS "
			"notes(id INTEGER PRIMARY KEY, note TEXT)";
	char *err = 0;
	sqlite3_exec_guard(db, stmt.c_str(), callback, nullptr, &err);
}

void munch::create_tags_table(sqlite3 *db)
{
	std::string stmt =
			"CREATE TABLE IF NOT EXISTS "
			"notes(id INTEGER PRIMARY KEY, note TEXT)";
	char *err = 0;
	sqlite3_exec_guard(db, stmt.c_str(), callback, nullptr, &err);
}

int munch::get_max_note_id(sqlite3 *db)
{
	std::string stmt = "SELECT MAX(id) FROM notes";
	char *err = 0;
	query_results qr;
	sqlite3_exec_guard(db, stmt.c_str(), callback, &qr, &err);
	return qr.id;
}

/* sqlite callback */
int munch::callback(void *res, int argc, char** argv, char** col)
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
			qr->text = std::string(argv[i]);
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

/* prepare an insert or replace statement for execution on the notes or tags table */
void munch::construct_insert(std::string& res, std::string verb,
							 std::string table, int id,
							 std::string text_field, int num_field)
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
	sanitise(text_field);
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

/* ensures a SQL sequence executes properly, or exits gracefully */
void munch::sqlite3_exec_guard(sqlite3 *db, std::string stmt,
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

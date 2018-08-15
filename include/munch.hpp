#ifndef MUNCH_HPP_
#define MUNCH_HPP_

#include <vector>
#include <sqlite3.h>

namespace munch {
	/* Stores options parsed from command line arguments */
	struct munch_options {
		std::string database = "munch.sqlite3";
	};

	/* Stores results from a sqlite3 query, used in callback functions */
	struct query_results {
		int id = 0;
		std::string text = "";
		std::vector<int> search_results {};
	};
	std::vector<int> search(sqlite3* db, std::vector<std::string> tags);
	void update_database(sqlite3* db, std::vector<std::string> files);
	int entry(int argc, char* argv[]);

	void sanitise(std::string& stmt);
	void create_notes_table(sqlite3 *db);
	void create_tags_table(sqlite3 *db);
	int get_max_note_id(sqlite3 *db);
	int callback(void *res, int argc, char** argv, char** col);
	void construct_insert(std::string& res, std::string verb, std::string table, int id, std::string text_field,int num_field=0);
	void sqlite3_exec_guard(sqlite3 *db, std::string stmt, int (*callback) (void*,int,char**,char**), void* res, char** err);
	munch_options parse_options(std::vector<std::string>& options);
	void print_usage_and_exit(bool help, std::string msg="");
	void print_error_and_exit(std::string msg="");
	void display_note(std::string db_path, int note_id);
	int parse_file(std::string f, std::vector<std::string>& tags,
				   std::string& contents);
}
#endif

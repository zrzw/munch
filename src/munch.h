#ifndef MUNCH_H_
#define MUNCH_H_
namespace munch{
  /* Holds options parsed from command line arguments */
    struct munch_options {
        std::string database = "munch.sqlite3";
    };

    struct query_results {
        int id = 0;
        std::string text = "";
        std::vector<int> search_results {};
    };
}
#endif

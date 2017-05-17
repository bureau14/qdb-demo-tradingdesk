#pragma once

#include <qdb/client.hpp>
#include <string>

qdb_error_t create_index_ts(qdb_handle_t h, const std::string & index);

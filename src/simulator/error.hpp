#pragma once

#include <qdb/client.hpp>

void throw_on_failure(qdb_error_t err, const char * msg);

struct connection_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

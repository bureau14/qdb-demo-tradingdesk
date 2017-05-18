#include "error.hpp"
#include <qdb/ts.h>
#include <fmt/format.h>

void throw_on_failure(qdb_error_t err, const char * msg)
{
    if (QDB_FAILURE(err))
    {
        fmt::print("Error: {} ({})\n", qdb_error(err), err);
        if ((QDB_ERROR_ORIGIN(err) == qdb_e_origin_connection) || (err == qdb_e_invalid_handle))
        {
            throw connection_error(err, msg);
        }
        else
        {
            throw std::runtime_error(msg);
        }
    }
}

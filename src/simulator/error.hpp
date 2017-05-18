#pragma once

#include <qdb/error.h>
#include <stdexcept>

void throw_on_failure(qdb_error_t err, const char * msg);

struct connection_error : std::runtime_error
{
    connection_error(qdb_error_t err, const char * msg) : std::runtime_error(msg), _code{err}
    {
    }

    qdb_error_t code() const
    {
        return _code;
    }

private:
    qdb_error_t _code;
};

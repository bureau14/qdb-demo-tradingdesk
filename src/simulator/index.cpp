#include "index.hpp"
#include <qdb/tag.h>
#include <qdb/ts.h>

qdb_error_t create_index_ts(qdb_handle_t h, const char * index)
{
    qdb_remove(h, index);

    qdb_ts_column_info_t column;
    column.name = "value";
    column.type = qdb_ts_column_double;

    qdb_error_t err = qdb_ts_create(h, index, &column, 1u);
    if (err) return err;

    err = qdb_attach_tag(h, index, "@indexes");
    if (err) return err;

    return qdb_attach_tag(h, "@indexes", "@tags");
}

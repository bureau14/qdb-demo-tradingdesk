#include "quote.hpp"

#include <qdb/ts.h>

qdb_error_t create_product_ts(qdb_handle_t h, const std::string & origin, const std::string & product)
{
    const std::string ts_name = origin + "." + product;

    qdb_remove(h, ts_name.c_str());

    qdb_ts_column_info_t columns[5];

    columns[0].name = "volume";
    columns[0].type = qdb_ts_column_double;

    columns[1].name = "open";
    columns[1].type = qdb_ts_column_double;

    columns[2].name = "high";
    columns[2].type = qdb_ts_column_double;

    columns[3].name = "low";
    columns[3].type = qdb_ts_column_double;

    columns[4].name = "close";
    columns[4].type = qdb_ts_column_double;

    return qdb_ts_create(h, ts_name.c_str(), columns, 5);
}

qdb_error_t insert_into_qdb(qdb_handle_t h, const quote & q)
{
    const std::string ts_name = q.origin + "." + q.symbol;

    qdb_ts_double_point dp;

    dp.timestamp = q.timestamp.as_timespec();

    dp.value = q.columns.volume;
    qdb_error_t err = qdb_ts_double_insert(h, ts_name.c_str(), "volume", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.open;
    err = qdb_ts_double_insert(h, ts_name.c_str(), "open", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.high;
    err = qdb_ts_double_insert(h, ts_name.c_str(), "high", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.low;
    err = qdb_ts_double_insert(h, ts_name.c_str(), "low", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.close;
    err = qdb_ts_double_insert(h, ts_name.c_str(), "close", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    return qdb_e_ok;
}
#include "quote.hpp"

#include <qdb/ts.h>


qdb_error_t insert_into_qdb(qdb_handle_t h, const quote & q)
{
    qdb_ts_double_point dp;

    dp.timestamp = q.timestamp.as_timespec();

    dp.value = q.columns.volume;
    qdb_error_t err = qdb_ts_double_insert(h, (q.origin + "." + q.symbol + ".volume").c_str(), &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.open;
    err = qdb_ts_double_insert(h, (q.origin + "." + q.symbol + ".open").c_str(), &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.high;
    err = qdb_ts_double_insert(h, (q.origin + "." + q.symbol + ".high").c_str(), &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.low;
    err = qdb_ts_double_insert(h, (q.origin + "." + q.symbol + ".low").c_str(), &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = q.columns.close;
    err = qdb_ts_double_insert(h, (q.origin + "." + q.symbol + ".close").c_str(), &dp, 1);
    if (QDB_FAILURE(err)) return err;

    return qdb_e_ok;
}
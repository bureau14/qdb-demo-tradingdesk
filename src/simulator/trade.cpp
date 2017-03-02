#include "trade.hpp"

#include <qdb/ts.h>

qdb_error_t insert_into_qdb(qdb_handle_t h, const trade & t)
{
    qdb_ts_blob_point bp;

    bp.timestamp = t.timestamp.as_timespec();
    bp.content = t.product.c_str();
    bp.content_length = t.product.size();

    qdb_error_t err = qdb_ts_blob_insert(h, (t.trader + ".product").c_str(), &bp, 1);
    if (QDB_FAILURE(err)) return err;

    bp.content = t.counterparty.c_str();
    bp.content_length = t.counterparty.size();

    err = qdb_ts_blob_insert(h, (t.trader + ".counterparty").c_str(), &bp, 1);
    if (QDB_FAILURE(err)) return err;
    
    qdb_ts_double_point dp;

    dp.timestamp = t.timestamp.as_timespec();
    dp.value = t.volume;
    
    err = qdb_ts_double_insert(h, (t.trader + ".volume").c_str(), &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = t.value;

    err = qdb_ts_double_insert(h, (t.trader + ".value").c_str(), &dp, 1);
    if (QDB_FAILURE(err)) return err;

    return qdb_e_ok;
}

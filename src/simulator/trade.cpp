#include "trade.hpp"
#include <qdb/tag.h>
#include <qdb/ts.h>

qdb_error_t create_trader_ts(qdb_handle_t h, const std::string & trader)
{
    qdb_remove(h, trader.c_str());

    qdb_ts_column_info_t columns[4];

    columns[0].name = "product";
    columns[0].type = qdb_ts_column_blob;

    columns[1].name = "counterparty";
    columns[1].type = qdb_ts_column_blob;

    columns[2].name = "volume";
    columns[2].type = qdb_ts_column_double;

    columns[3].name = "value";
    columns[3].type = qdb_ts_column_double;

    auto err = qdb_ts_create(h, trader.c_str(), columns, 4);
    if (QDB_SUCCESS(err))
    {
        static const char * tag = "@traders";

        qdb_attach_tag(h, trader.c_str(), tag);

        err = qdb_attach_tag(h, tag, "@tags");
    }
    return err;
}

qdb_error_t insert_into_qdb(qdb_handle_t h, const trade & t)
{
    qdb_ts_blob_point bp;

    bp.timestamp = t.timestamp.as_timespec();
    bp.content = t.product.c_str();
    bp.content_length = t.product.size();

    qdb_error_t err = qdb_ts_blob_insert(h, t.trader.c_str(), "product", &bp, 1);
    if (QDB_FAILURE(err)) return err;

    bp.content = t.counterparty.c_str();
    bp.content_length = t.counterparty.size();

    err = qdb_ts_blob_insert(h, t.trader.c_str(), "counterparty", &bp, 1);
    if (QDB_FAILURE(err)) return err;

    qdb_ts_double_point dp;

    dp.timestamp = t.timestamp.as_timespec();
    dp.value = t.volume;

    err = qdb_ts_double_insert(h, t.trader.c_str(), "volume", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = t.value;

    err = qdb_ts_double_insert(h, t.trader.c_str(), "value", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    return qdb_e_ok;
}

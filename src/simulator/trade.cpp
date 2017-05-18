#include "trade.hpp"
#include <qdb/tag.h>
#include <qdb/ts.h>
#include <array>
#include <iostream>

static qdb_error_t create_ts(qdb_handle_t h, const std::string & timeseries, const char * first_column)
{
    qdb_remove(h, timeseries.c_str());

    std::array<qdb_ts_column_info_t, 4> columns;

    columns[0].name = first_column;
    columns[0].type = qdb_ts_column_blob;

    columns[1].name = "counterparty";
    columns[1].type = qdb_ts_column_blob;

    columns[2].name = "volume";
    columns[2].type = qdb_ts_column_double;

    columns[3].name = "value";
    columns[3].type = qdb_ts_column_double;

    return qdb_ts_create(h, timeseries.c_str(), columns.data(), columns.size());
}

qdb_error_t create_trader_ts(qdb_handle_t h, const std::string & trader)
{
    auto err = create_ts(h, trader, /*first_column=*/"product");
    if (QDB_SUCCESS(err))
    {
        static const char * tag = "@traders";

        err = qdb_attach_tag(h, trader.c_str(), tag);
        qdb_attach_tag(h, tag, "@tags");
    }
    return err;
}

qdb_error_t create_product_ts(qdb_handle_t h, const std::string & product)
{
    return create_ts(h, product, /*first_column=*/"trader");
}

static qdb_error_t insert_volume_and_value(qdb_handle_t h, const trade & t, const char * timeseries)
{
    qdb_ts_double_point dp;
    dp.timestamp = t.timestamp.as_timespec();
    dp.value = t.volume;

    auto err = qdb_ts_double_insert(h, timeseries, "volume", &dp, 1);
    if (QDB_FAILURE(err)) return err;

    dp.value = t.value;

    err = qdb_ts_double_insert(h, timeseries, "value", &dp, 1);
    return err;
}

static qdb_error_t insert_counterparty(qdb_handle_t h, const trade & t, const char * timeseries)
{
    qdb_ts_blob_point bp;
    bp.timestamp = t.timestamp.as_timespec();
    bp.content = t.counterparty.c_str();
    bp.content_length = t.counterparty.size();

    return qdb_ts_blob_insert(h, timeseries, "counterparty", &bp, 1);
}

static qdb_error_t insert_product(qdb_handle_t h, const trade & t, const char * timeseries)
{
    qdb_ts_blob_point bp;
    bp.timestamp = t.timestamp.as_timespec();
    bp.content = t.product.c_str();
    bp.content_length = t.product.size();

    return qdb_ts_blob_insert(h, timeseries, "product", &bp, 1);
}

static qdb_error_t insert_trader(qdb_handle_t h, const trade & t, const char * timeseries)
{
    qdb_ts_blob_point bp;
    bp.timestamp = t.timestamp.as_timespec();
    bp.content = t.trader.c_str();
    bp.content_length = t.trader.size();

    return qdb_ts_blob_insert(h, timeseries, "trader", &bp, 1);
}

static qdb_error_t insert_into_trader(qdb_handle_t h, const trade & t)
{
    const auto timeseries = t.trader.c_str();

    qdb_error_t err = insert_product(h, t, timeseries);
    if (QDB_FAILURE(err)) return err;

    err = insert_counterparty(h, t, timeseries);
    if (QDB_FAILURE(err)) return err;

    return insert_volume_and_value(h, t, timeseries);
}

static qdb_error_t insert_into_product(qdb_handle_t h, const trade & t)
{
    const auto timeseries = t.product.c_str();

    qdb_error_t err = insert_trader(h, t, timeseries);
    if (QDB_FAILURE(err)) return err;

    err = insert_counterparty(h, t, timeseries);
    if (QDB_FAILURE(err)) return err;

    return insert_volume_and_value(h, t, timeseries);
}

static qdb_error_t update_index(qdb_handle_t h, const trade & t, const products & prods)
{
    static const double dow_divisor = 0.14602128057775;
    static std::unordered_map<std::string, double> current_values;

    const product & p = prods.at(t.product);

    qdb_ts_double_aggregation_t agg;
    agg.type = qdb_agg_last;
    agg.range.begin.tv_sec = 0;
    agg.range.end.tv_sec = std::numeric_limits<qdb_time_t>::max();
    agg.result.value = 0;

    qdb_error_t err = qdb_ts_double_aggregate(h, p.index, "value", &agg, 1u);
    // FIXME(marek): double_aggregate should not result in alias_not_found here!
    if (QDB_FAILURE(err))
    {
        if (err != qdb_e_alias_not_found) return err;

        agg.result.value = 0;
    }

    auto new_index = agg.result.value + (-current_values[t.product] + t.value) / dow_divisor;
    if (std::isnan(new_index))
    {
        std::cerr << "warning: resetting index\n";
        new_index = 0;
        current_values.clear();
    }

    current_values[t.product] = t.value;

    qdb_ts_double_point dp;
    dp.timestamp = t.timestamp.as_timespec();
    dp.value = new_index;

    err = qdb_ts_double_insert(h, p.index, "value", &dp, 1);
    return err;
}

qdb_error_t insert_into_qdb(qdb_handle_t h, const trade & t, const products & prods)
{
    auto err = insert_into_trader(h, t);
    if (QDB_FAILURE(err)) return err;

    err = insert_into_product(h, t);
    if (QDB_FAILURE(err)) return err;

    err = update_index(h, t, prods);
    return err;
}

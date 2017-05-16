#pragma once

#include <utils/timespec.hpp>
#include <string>

#include "quote.hpp"

struct trade
{
    trade(const std::string & t = {}, const quote & q = {})
        : timestamp{utils::timestamp()}, trader{t},
          counterparty{q.origin}, product{q.symbol}, volume{q.columns.volume}, value{q.columns.close}
    {
    }

    utils::timespec timestamp;

    std::string trader;
    std::string counterparty;

    std::string product;

    double volume{0.0};
    double value{0.0};
};

qdb_error_t create_trader_ts(qdb_handle_t h, const std::string & trader);
qdb_error_t insert_into_qdb(qdb_handle_t h, const trade & t);

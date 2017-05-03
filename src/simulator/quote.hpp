#pragma once

#include <qdb/ts.h>

#include <utils/time.hpp>
#include <utils/timespec.hpp>
#include <string>

#include <vector>

struct quote
{
    quote()
    {
    }

    template <typename Generator>
    quote(const std::string & o, const std::string & s, std::uint32_t vol, Generator & g)
        : timestamp{utils::timestamp()}, origin{o}, symbol{s}, columns{g(vol)}
    {
    }

    utils::timespec timestamp;

    std::string origin;
    std::string symbol;

    struct values
    {
        values(double v = 0.0, double o = 0.0, double h = 0.0, double l = 0.0, double c = 0.0)
            : volume{v}, open{o}, high{h}, low{l}, close{c}
        {
        }

        double volume;
        double open;
        double high;
        double low;
        double close;
    };

    values columns;
};

struct quotes_in_cols
{
    quotes_in_cols(const std::string & o, const std::string & s) : origin{ o }, symbol{ s } {}
        
    template <typename Generator>
    utils::timespec generate(Generator & g, const utils::timespec & start_at, std::uint64_t count)
    {
        volumes.resize(count);
        opens.resize(count);
        highs.resize(count);
        lows.resize(count);
        closes.resize(count);

        qdb_timespec_t current_time = start_at.as_timespec();

        for (std::uint64_t i = 0; i < count; ++i, ++current_time.tv_sec)
        {
            quote::values v = g();

            volumes[i].timestamp = current_time;
            volumes[i].value = v.volume;

            opens[i].timestamp = current_time;
            opens[i].value = v.open;

            highs[i].timestamp = current_time;
            highs[i].value = v.high;

            lows[i].timestamp = current_time;
            lows[i].value = v.low;

            closes[i].timestamp = current_time;
            closes[i].value = v.close;
        }

        return utils::timespec{ current_time };
    }

    std::string origin;
    std::string symbol;

    std::vector<qdb_ts_double_point> volumes;
    std::vector<qdb_ts_double_point> opens;
    std::vector<qdb_ts_double_point> highs;
    std::vector<qdb_ts_double_point> lows;
    std::vector<qdb_ts_double_point> closes;
};

qdb_error_t create_product_ts(qdb_handle_t h, const std::string & origin, const std::string & product);
qdb_error_t insert_into_qdb(qdb_handle_t h, const quotes_in_cols & q_cols);
qdb_error_t insert_into_qdb(qdb_handle_t h, const quote & q);

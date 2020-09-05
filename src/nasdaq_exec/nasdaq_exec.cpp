#include "itch_exec.hpp"
#include <qdb/client.hpp>
#include <qdb/integer.h>
#include <qdb/query.h>
#include <qdb/tag.h>
#include <qdb/ts.h>
#include <boost/container/flat_map.hpp>
#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/locale.h>
#include <utils/gregorian.hpp>
#include <utils/stringify.hpp>
#include <clocale>

struct config
{
    bool collapsed;
    bool point_in_time;
    std::string qdb_url;
    std::string stock;
    std::string when;
};

static void throw_on_failure(qdb_error_t err, const char * msg)
{
    if (QDB_FAILURE(err))
    {
        fmt::print(fmt::fg(fmt::color::red), "Error: {} ({})\n", qdb_error(err), err);
        throw std::runtime_error(msg);
    }
}

static std::pair<utils::timespec, utils::timespec> get_time_range(const std::string & when)
{
    const auto time_point = utils::from_iso_extended_string_utc(when);
    if (time_point == std::chrono::system_clock::time_point{}) throw std::runtime_error("invalid time");

    const auto time_point_ts = utils::timespec{std::chrono::duration_cast<std::chrono::seconds>(time_point.time_since_epoch())};
    const auto requested_day = utils::extract_date(time_point_ts);

    return std::make_pair(utils::make_timespec(requested_day), time_point_ts);
}

static config parse_config(int argc, char ** argv)
{
    config cfg;

    boost::program_options::options_description desc{"Allowed options"};
    desc.add_options()                                                                                           //
        ("help,h", "show help message")                                                                          //
        ("url", boost::program_options::value<std::string>(&cfg.qdb_url)->default_value("qdb://127.0.0.1:2836")) //
        ("stock", boost::program_options::value<std::string>(&cfg.stock))                                        //
        ("when", boost::program_options::value<std::string>(&cfg.when))                                          //
        ("collapsed", boost::program_options::value<bool>(&cfg.collapsed)->default_value(false))                 //
        ("point-in-time", boost::program_options::value<bool>(&cfg.point_in_time)->default_value(true))          //
        ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        std::exit(0);
    }

    if (cfg.stock.empty())
    {
        throw std::runtime_error("please specify a stock");
    }

    if (cfg.when.empty())
    {
        throw std::runtime_error("please specify a point in time");
    }

    return cfg;
}

struct ts_int64
{
    qdb_ts_int64_point * points;
    size_t count;
};

struct ts_double
{
    qdb_ts_double_point * points;
    size_t count;
};

ts_int64 get_ranges_int64(qdb_handle_t h, const std::string & table_name, const std::string & column, const qdb_ts_range_t & ranges)
{
    ts_int64 res;

    auto err = qdb_ts_int64_get_ranges(h, table_name.c_str(), column.c_str(), &ranges, 1u, &res.points, &res.count);
    throw_on_failure(err, "int64 get range");

    return res;
}

ts_double get_ranges_double(qdb_handle_t h, const std::string & table_name, const std::string & column, const qdb_ts_range_t & ranges)
{
    ts_double res;

    auto err = qdb_ts_double_get_ranges(h, table_name.c_str(), column.c_str(), &ranges, 1u, &res.points, &res.count);
    throw_on_failure(err, "int64 get range");

    return res;
}

static itch::order_records get_orders_records(qdb_handle_t h, const std::string & table_name, utils::timespec first, utils::timespec last)
{
    qdb_ts_range_t r;

    r.begin = first.as_timespec();
    r.end   = last.as_timespec();

    ts_int64 order_type               = get_ranges_int64(h, table_name, "type", r);
    ts_int64 order_reference          = get_ranges_int64(h, table_name, "reference", r);
    ts_int64 order_original_reference = get_ranges_int64(h, table_name, "original_reference", r);
    ts_int64 order_new_reference      = get_ranges_int64(h, table_name, "new_reference", r);
    ts_int64 order_is_buy             = get_ranges_int64(h, table_name, "is_buy", r);
    ts_int64 order_shares             = get_ranges_int64(h, table_name, "shares", r);
    ts_double order_price             = get_ranges_double(h, table_name, "price", r);

    itch::order_records result;

    result.reserve(order_type.count);

    // TODO: check all columns
    if (order_type.count != order_reference.count) throw std::runtime_error("incoherence column count");

    for (size_t i = 0; i < order_type.count; ++i)
    {
        auto it = result.emplace_hint(result.end(), utils::timespec{order_type.points[i].timestamp}, itch::order_record{});

        itch::order_record & new_rec = it->second;

        // type
        new_rec.order_type = static_cast<char>(order_type.points[i].value);

        // reference
        new_rec.reference = static_cast<std::uint64_t>(order_reference.points[i].value);

        // original reference
        if (order_original_reference.points[i].value != qdb_int64_undefined)
        {
            assert(new_rec.reference == qdb_int64_undefined);
            new_rec.reference = static_cast<std::uint64_t>(order_original_reference.points[i].value);
        }

        // new reference
        new_rec.new_reference = static_cast<std::uint64_t>(order_new_reference.points[i].value);

        // is buy
        new_rec.is_buy = static_cast<bool>(order_is_buy.points[i].value);

        // shares
        new_rec.shares = static_cast<std::uint32_t>(order_shares.points[i].value);

        // price
        new_rec.price = order_price.points[i].value;
    }

    qdb_release(h, order_type.points);
    qdb_release(h, order_reference.points);
    qdb_release(h, order_original_reference.points);
    qdb_release(h, order_new_reference.points);
    qdb_release(h, order_is_buy.points);
    qdb_release(h, order_shares.points);
    qdb_release(h, order_price.points);

    return result;
}

static std::string build_bar(std::string::size_type max_length, std::uint32_t current_value, std::uint32_t max_value)
{
    if (current_value >= max_value)
    {
        return std::string(max_length, '#');
    }

    auto l = static_cast<std::string::size_type>(
        static_cast<double>(max_length) * static_cast<double>(current_value) / static_cast<double>(max_value));
    return std::string(l, '#');
}

static utils::timespec get_snapshot_timestamp(utils::timespec when) noexcept
{
    when.sec  = when.sec - (when.sec % std::chrono::minutes{15});
    when.nsec = std::chrono::nanoseconds{};
    return when;
}

static std::string make_snapshot_key(const std::string & stock, utils::timespec when)
{
    // modulo 15 mins, remove nsec, remove the 900 secs
    when = get_snapshot_timestamp(when);

    return fmt::format("{}_orders_snap_{}", stock, utils::to_iso_extended_string_utc(static_cast<std::time_t>(when.sec.count())));
}

static utils::timespec restore_snapshot(qdb_handle_t h, itch::execution_engine & engine, const std::string & stock, utils::timespec when)
{
    // we snapshot every 15 minutes, for the day, get all snapshots
    const auto requested_day = utils::extract_date(when);

    // build the prefix
    const auto best_snap_key = make_snapshot_key(stock, when);
    const auto prefix        = fmt::format("{}_orders_snap_{:04}-{:02}-{:02}T", stock, static_cast<int>(requested_day.year()),
        static_cast<int>(requested_day.month()), static_cast<int>(requested_day.day()));

    const char ** results = 0;
    size_t results_count  = 0;

    auto err = qdb_prefix_get(h, prefix.c_str(), 100, &results, &results_count);
    if (err == qdb_e_alias_not_found) return utils::timespec{};
    throw_on_failure(err, "cannot prefix get");

    if (!results_count) return utils::timespec{};

    const char * match = nullptr;

    // get the first workable timestamp
    // we could do a binary search
    for (size_t i = 0; i < results_count; ++i)
    {
        if (strcmp(results[i], best_snap_key.c_str()) > 0) break;
        match = results[i];
    }

    const auto snap_key = std::string{match};
    size_t idx          = snap_key.rfind('_');
    if (idx == std::string::npos) return utils::timespec{};

    qdb_release(h, results);
    if (!match) return utils::timespec{};

    const void * snap_data = nullptr;
    qdb_size_t snap_size   = 0;

    err = qdb_blob_get(h, snap_key.c_str(), &snap_data, &snap_size);
    throw_on_failure(err, "cannot get snapshot");

    const std::uint8_t * p = static_cast<const std::uint8_t *>(snap_data);

    auto deserialized = engine.deserialize_state(p, snap_size);

    qdb_release(h, snap_data);

    if (!deserialized) return utils::timespec{};

    const auto tp = utils::from_iso_extended_string_utc(snap_key.substr(idx + 1u));
    if (tp == std::chrono::system_clock::time_point{}) return utils::timespec{};

    return utils::timespec{std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch())};
}

static void store_snapshot(qdb_handle_t h, const itch::execution_engine & engine, const std::string & stock, utils::timespec when)
{
    when                                      = get_snapshot_timestamp(when);
    const auto snap_key                       = make_snapshot_key(stock, when);
    std::vector<std::uint8_t> serialized_snap = engine.serialize_state();
    auto err = qdb_blob_update(h, snap_key.data(), serialized_snap.data(), serialized_snap.size(), qdb_never_expires);
    throw_on_failure(err, "cannot store snapshot");
}

static void print_books(const itch::order_book & buying_book, const itch::order_book & selling_book)
{
    if (buying_book.empty() || selling_book.empty()) return;

    fmt::print(fmt::fg(fmt::color::cyan), "\nDETAILED ORDER BOOK \n");

    fmt::print(fmt::fg(fmt::color::orange), "\n *** Selling\n");

    auto it_max_sell = std::max_element(selling_book.cbegin(), selling_book.cend(),
        [](const auto & left, const auto & right) { return left.first.shares < right.first.shares; });
    auto it_max_buy  = std::max_element(buying_book.cbegin(), buying_book.cend(),
        [](const auto & left, const auto & right) { return left.first.shares < right.first.shares; });

    const auto the_max = std::max(it_max_sell->first.shares, it_max_buy->first.shares);

    for (auto it = selling_book.crbegin(); it != selling_book.crend(); ++it)
    {
        fmt::print(fmt::fg(fmt::color::orange), " {:>9.2f} USD - {:>6L} shares - ref: {:>10L} | {}\n",
            static_cast<double>(it->first.price) / 1000.0, it->first.shares, it->second, build_bar(20, it->first.shares, the_max));
    }

    fmt::print(fmt::fg(fmt::color::green), "\n *** Buying\n");

    for (auto it = buying_book.crbegin(); it != buying_book.crend(); ++it)
    {
        fmt::print(fmt::fg(fmt::color::green), " {:>9.2f} USD - {:>6L} shares - ref: {:>10L} | {}\n",
            static_cast<double>(it->first.price) / 1000.0, it->first.shares, it->second, build_bar(20, it->first.shares, the_max));
    }
}

static void print_collapsed_books(const itch::collapsed_book & buying_book, const itch::collapsed_book & selling_book)
{
    if (buying_book.empty() || selling_book.empty()) return;

    fmt::print(fmt::fg(fmt::color::cyan), "\nCOLLAPSED ORDER BOOK \n");

    fmt::print(fmt::fg(fmt::color::orange), "\n *** Selling\n");

    auto it_max_sell = std::max_element(
        selling_book.cbegin(), selling_book.cend(), [](const auto & left, const auto & right) { return left.second < right.second; });
    auto it_max_buy = std::max_element(
        buying_book.cbegin(), buying_book.cend(), [](const auto & left, const auto & right) { return left.second < right.second; });

    const auto the_max = std::max(it_max_sell->second, it_max_buy->second);

    for (auto it = selling_book.crbegin(); it != selling_book.crend(); ++it)
    {
        fmt::print(fmt::fg(fmt::color::orange), " {:>9.2f} USD - {:>6L} shares | {}\n", static_cast<double>(it->first) / 1000.0, it->second,
            build_bar(20, it->second, the_max));
    }

    fmt::print(fmt::fg(fmt::color::green), "\n *** Buying\n");

    for (auto it = buying_book.crbegin(); it != buying_book.crend(); ++it)
    {
        fmt::print(fmt::fg(fmt::color::green), " {:>9.2f} USD - {:>6L} shares | {}\n", static_cast<double>(it->first) / 1000.0, it->second,
            build_bar(20, it->second, the_max));
    }
}

int main(int argc, char ** argv)
{

    try
    {
        std::locale::global(std::locale("en_US.UTF-8"));
        std::setlocale(LC_ALL, "en_US.UTF-8");

        fmt::print("Nasdaq orders executor\n");

        const config cfg = parse_config(argc, argv);

        qdb::handle h;

        qdb_error_t err = h.connect(cfg.qdb_url.c_str());
        throw_on_failure(err, "connection error");

        auto total_start_time = std::chrono::high_resolution_clock::now();

        const auto [range_start_ts, range_end_ts] = get_time_range(cfg.when);

        itch::order_records orders;
        itch::execution_engine engine;

        std::uint64_t missed_orders = 0;

        itch::order_records::const_iterator it_orders_records;
        utils::timespec snap_ts;

        if (cfg.point_in_time)
        {
            // look for a snapshot
            snap_ts = restore_snapshot(h, engine, cfg.stock, range_end_ts);
        }

        if (snap_ts != utils::timespec{})
        {
            fmt::print("Used snapshot {}\n", utils::to_iso_extended_string_utc(static_cast<std::time_t>(snap_ts.sec.count())));
            orders            = get_orders_records(h, cfg.stock + "_orders", snap_ts, range_end_ts);
            it_orders_records = orders.lower_bound(snap_ts);
        }
        else
        {
            orders            = get_orders_records(h, cfg.stock + "_orders", range_start_ts, range_end_ts);
            it_orders_records = orders.cbegin();
        }

        auto get_end_time = std::chrono::high_resolution_clock::now();

        for (; it_orders_records != orders.cend(); ++it_orders_records)
        {
            if (!engine.run_order(it_orders_records->second)) ++missed_orders;
        }

        if (cfg.point_in_time && !orders.empty() && (snap_ts < get_snapshot_timestamp(range_end_ts)))
        {
            store_snapshot(h, engine, cfg.stock, range_end_ts);
        }

        auto engine_end_time = std::chrono::high_resolution_clock::now();

        const auto buying_book  = engine.buy_book();
        const auto selling_book = engine.sell_book();

        auto total_end_time = std::chrono::high_resolution_clock::now();

        fmt::print(
            "Order book for {} at {} \n", cfg.stock, utils::to_iso_extended_string_utc(static_cast<std::time_t>(range_end_ts.sec.count())));
        fmt::print("Processed orders {:L} - missed orders {:L} - Point In Time: {}\n", orders.size(), missed_orders,
            cfg.point_in_time ? "enabled" : "disabled");

        if (cfg.collapsed)
        {
            const auto collapsed_buying_book  = engine.collapse_book(buying_book);
            const auto collapsed_selling_book = engine.collapse_book(selling_book);

            print_collapsed_books(collapsed_buying_book, collapsed_selling_book);
        }
        else
        {
            print_books(buying_book, selling_book);
        }

        const auto elapsed_get   = std::chrono::duration_cast<std::chrono::microseconds>(get_end_time - total_start_time);
        const auto elapsed_run   = std::chrono::duration_cast<std::chrono::microseconds>(engine_end_time - get_end_time);
        const auto build_book    = std::chrono::duration_cast<std::chrono::microseconds>(total_end_time - engine_end_time);
        const auto total_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(total_end_time - total_start_time);

        fmt::print(fmt::fg(fmt::color::cyan), "\n Total elapsed time: {:>9L} us\n", total_elapsed.count());
        fmt::print(fmt::fg(fmt::color::cyan), "      Data transfer: {:>9L} us\n", elapsed_get.count());
        fmt::print(fmt::fg(fmt::color::cyan), "   Engine execution: {:>9L} us\n", elapsed_run.count());
        fmt::print(fmt::fg(fmt::color::cyan), "      Book building: {:>9L} us\n", build_book.count());

        return EXIT_SUCCESS;
    }

    catch (const boost::program_options::validation_error & e)
    {
        fmt::print(fmt::fg(fmt::color::red), "invalid option: {}", e.what());
        return EXIT_FAILURE;
    }

    catch (const std::error_code & ec)
    {
        fmt::print(fmt::fg(fmt::color::red), "error caught: {}", ec.message());
        return EXIT_FAILURE;
    }

    catch (const std::exception & e)
    {
        fmt::print(fmt::fg(fmt::color::red), "exception caught: {}", e.what());
        return EXIT_FAILURE;
    }
}

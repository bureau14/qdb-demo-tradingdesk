#include <qdb/client.hpp>
#include <qdb/integer.h>
#include <qdb/query.h>
#include <qdb/tag.h>
#include <qdb/ts.h>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/program_options.hpp>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <utils/gregorian.hpp>
#include <iostream>
#include <random>
#include <thread>
#include <tuple>

static constexpr std::uint64_t min_iterations = 8;

struct config
{
    std::string qdb_url;
    unsigned int seed;
    std::uint32_t days;
    std::uint16_t year;
    std::uint64_t errors;
    std::uint64_t iterations;
};

static void throw_on_failure(qdb_error_t err, const char * msg)
{
    if (QDB_FAILURE(err))
    {
        fmt::print(fmt::fg(fmt::color::red), "Error: {} ({})\n", qdb_error(err), err);
        throw std::runtime_error(msg);
    }
}
static config parse_config(int argc, char ** argv)
{
    config cfg;

    boost::program_options::options_description desc{"Allowed options"};
    desc.add_options()                                                                                               //
        ("help,h", "show help message")                                                                              //
        ("url", boost::program_options::value<std::string>(&cfg.qdb_url)->default_value("qdb://127.0.0.1:2836"))     //
        ("seed", boost::program_options::value<unsigned int>(&cfg.seed)->default_value(0))                           //
        ("days", boost::program_options::value<std::uint32_t>(&cfg.days)->default_value(365))                        //
        ("year", boost::program_options::value<std::uint16_t>(&cfg.year)->default_value(2008))                       //
        ("iterations", boost::program_options::value<std::uint64_t>(&cfg.iterations)->default_value(min_iterations)) //
        ("errors", boost::program_options::value<std::uint64_t>(&cfg.errors)->default_value(10))                     //
        ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    boost::program_options::notify(vm);

    if (!cfg.seed)
    {
        std::random_device rnd;
        cfg.seed = rnd();
    }

    if (cfg.iterations < min_iterations)
    {
        std::cerr << "iterations must be greater than " << min_iterations << std::endl;
        std::exit(1);
    }

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        std::exit(0);
    }

    return cfg;
}

template <typename Iterator>
static void create_entries(qdb_handle_t h, const char * prefix, Iterator first, Iterator last)
{
    int i = 0;

    for (Iterator it = first; it != last; ++it, ++i)
    {
        const std::string key_name = fmt::format("{}.{}", prefix, i);
        throw_on_failure(qdb_blob_update(h, key_name.c_str(), it->data(), it->size(), qdb_never_expires), "could not create/update entry");
    }
}

static const std::vector<std::string> agents = {"bob", "alice", "oscar"};

static void create_agents(qdb_handle_t h)
{
    create_entries(h, "agent", agents.cbegin(), agents.cend());
}

static const std::vector<std::string> counterparties = {"burger", "borscht", "sashimi", "baguette", "kaese", "dumpling"};

static void create_counterparties(qdb_handle_t h)
{
    create_entries(h, "counterparty", counterparties.cbegin(), counterparties.cend());
}

static const std::vector<std::string> securities = {"mmm", "axp", "aapl", "ba", "cat", "cvx", "csco", "ko", "dis", "xom", "ge", "gs", "hd",
    "ibm", "intc", "jnj", "jpm", "mcd", "mrk", "msft", "nke", "pfe", "pg", "trv", "utx", "unh", "vz", "v", "wmt", "dwdp"};

static const std::vector<double> securities_start_price = {200.0, 100.0, 170.0, 400.0, 120.0, 120.0, 50.0, 50.0, 110.0, 70.0, 10.0, 190.0,
    180.0, 130.0, 50.0, 130.0, 100.0, 170.0, 77.0, 100.0, 80.0, 40.0, 10.0, 120.0, 120.0, 260.0, 50.0, 140.0, 90.0, 990.0};

static void create_securities(qdb_handle_t h)
{
    create_entries(h, "security", securities.cbegin(), securities.cend());
}

static const char * const table_name = "tlog";

static void create_table(qdb_handle_t h)
{
    qdb_entry_metadata_t md;

    // already exists, remove
    if (qdb_get_metadata(h, table_name, &md) == qdb_e_ok)
    {
        throw_on_failure(qdb_remove(h, table_name), "could not remove table");
    }

    std::vector<qdb_ts_column_info_t> columns(5);

    columns[0].name = "security";
    columns[0].type = qdb_ts_column_int64;

    columns[1].name = "price";
    columns[1].type = qdb_ts_column_double;

    columns[2].name = "volume";
    columns[2].type = qdb_ts_column_int64;

    columns[3].name = "agent";
    columns[3].type = qdb_ts_column_int64;

    columns[4].name = "counterparty";
    columns[4].type = qdb_ts_column_int64;

    throw_on_failure(qdb_ts_create(h, table_name, qdb_d_day, columns.data(), columns.size()), "cannot create table");
}

static void init_batch_table(qdb_handle_t h, const config & cfg, qdb_batch_table_t * b)
{
    std::vector<qdb_ts_batch_column_info_t> columns(5);

    columns[0].timeseries          = table_name;
    columns[0].column              = "security";
    columns[0].elements_count_hint = cfg.iterations;

    columns[1].timeseries          = table_name;
    columns[1].column              = "price";
    columns[1].elements_count_hint = cfg.iterations;

    columns[2].timeseries          = table_name;
    columns[2].column              = "volume";
    columns[2].elements_count_hint = cfg.iterations;

    columns[3].timeseries          = table_name;
    columns[3].column              = "agent";
    columns[3].elements_count_hint = cfg.iterations;

    columns[4].timeseries          = table_name;
    columns[4].column              = "counterparty";
    columns[4].elements_count_hint = cfg.iterations;

    throw_on_failure(qdb_ts_batch_table_init(h, columns.data(), columns.size(), b), "cannot create batch");
}

class security_generator
{
public:
    security_generator() = default;
    explicit security_generator(double l)
        : _last{l}
        , _increment{-(_last / 10000.0), _last / 10000.0}
    {}

public:
    template <typename Generator>
    double operator()(Generator & g) noexcept
    {
        _last = std::max(_last, _last + _increment(g));
        return _last;
    }

private:
    double _last{0.0};
    std::normal_distribution<double> _increment;
};

struct generator_state
{
    static constexpr std::int64_t min_volume = 100;
    static constexpr std::int64_t max_volume = 10'000;

    explicit generator_state(unsigned int seed) noexcept
        : _generator{seed}
        , _agent{std::int64_t{0}, static_cast<std::int64_t>(agents.size() - 1)}
        , _counterparty{std::int64_t{0}, static_cast<std::int64_t>(counterparties.size() - 1)}
        , _security{std::int64_t{0}, static_cast<std::int64_t>(securities.size() - 1)}
        , _max_volume{max_volume - 1'000, max_volume + 1'000}
        , _volume{std::int64_t{1}, max_volume / min_volume}
    {
        _securities.reserve(securities.size());

        for (double p : securities_start_price)
        {
            _securities.emplace_back(p);
        }

        assert(_securities.size() == securities.size());

        randomize_max_volume();
    }

    generator_state(const generator_state & /*other*/) = delete;

public:
    void randomize_max_volume()
    {
        _volume = std::uniform_int_distribution<std::int64_t>{min_volume, _max_volume(_generator)};
    }

public:
    std::tuple<std::int64_t, double> security() noexcept
    {
        const std::int64_t s = _security(_generator);
        return std::make_pair(s, _securities[s](_generator));
    }

    std::int64_t volume() noexcept
    {
        return min_volume * _volume(_generator);
    }

    std::uint64_t error_volume() noexcept
    {
        return volume() * std::uint64_t{100'000};
    }

    std::int64_t agent() noexcept
    {
        return _agent(_generator);
    }

    std::int64_t error_agent() noexcept
    {
        return agents.size() - 1u;
    }

    std::int64_t counterparty() noexcept
    {
        return _counterparty(_generator);
    }

public:
    std::uint64_t iterations_count(std::uint64_t max_iterations) noexcept
    {
        if (max_iterations < 20) return max_iterations;

        std::uniform_int_distribution<std::uint64_t> dis{max_iterations - (max_iterations / 10), max_iterations};
        return dis(_generator);
    }

private:
    std::minstd_rand _generator;

    std::uniform_int_distribution<std::int64_t> _agent;
    std::uniform_int_distribution<std::int64_t> _counterparty;
    std::uniform_int_distribution<std::int64_t> _security;
    std::uniform_int_distribution<std::int64_t> _max_volume;
    std::uniform_int_distribution<std::int64_t> _volume;

    std::vector<security_generator> _securities;
};

static void generate_transaction(qdb_batch_table_t b, const utils::timespec & ts, generator_state & gen)
{
    qdb_timespec_t qts = ts.as_timespec();

    throw_on_failure(qdb_ts_batch_start_row(b, &qts), "cannot start new row");

    std::int64_t s{0};
    double v{0.0};

    std::tie(s, v) = gen.security();

    qdb_size_t index = 0;

    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, s), "cannot set security");
    throw_on_failure(qdb_ts_batch_row_set_double(b, index++, v), "cannot set security value");
    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, gen.volume()), "cannot set volume");
    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, gen.agent()), "cannot set agent");
    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, gen.counterparty()), "cannot set counterparty");
}

static void generate_error_transaction(qdb_batch_table_t b, const utils::timespec & ts, generator_state & gen)
{
    qdb_timespec_t qts = ts.as_timespec();

    throw_on_failure(qdb_ts_batch_start_row(b, &qts), "cannot start new row");

    std::int64_t s{0};
    double v{0.0};

    std::tie(s, v) = gen.security();

    qdb_size_t index = 0;

    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, s), "cannot set security");
    throw_on_failure(qdb_ts_batch_row_set_double(b, index++, v), "cannot set security value");
    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, gen.error_volume()), "cannot set volume");
    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, gen.error_agent()), "cannot set agent");
    throw_on_failure(qdb_ts_batch_row_set_int64(b, index++, gen.counterparty()), "cannot set counterparty");
}

static void skip_weekend(boost::gregorian::day_iterator & it_day)
{
    // point to Monday
    if (it_day->day_of_week() == 0)
    {
        ++it_day;
    }
    else if (it_day->day_of_week() == 6)
    {
        ++it_day;
        ++it_day;
    }
}

static void push_to_db(qdb_batch_table_t b)
{
    const auto start = std::chrono::high_resolution_clock::now();

    throw_on_failure(qdb_ts_batch_push(b), "cannot push batch");

    const auto stop = std::chrono::high_resolution_clock::now();

    fmt::print("Pushed to the database in {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start));
}

static bool should_generate_error(const config & cfg, std::uint32_t day, std::uint64_t i)
{
    // generate 10 errors somehow in the middle

    if (day != (cfg.days / 2)) return false;

    if (cfg.iterations < cfg.errors) return true;

    if ((cfg.iterations / 2) < cfg.errors)
    {
        return i < cfg.errors;
    }

    const std::uint64_t start_error = (cfg.iterations / 2) - cfg.errors;
    const std::uint64_t end_error   = start_error + cfg.errors;

    return (i >= start_error) && (i < end_error);
}

static void generate_transaction_log(qdb_handle_t h, const config & cfg)
{
    const auto global_start = std::chrono::high_resolution_clock::now();

    qdb_batch_table_t b;

    init_batch_table(h, cfg, &b);

    generator_state gen{cfg.seed};

    boost::gregorian::date start_day{cfg.year, 1, 1};

    boost::gregorian::day_iterator it_day{start_day};

    for (std::uint32_t days = 0; days < cfg.days; ++days, ++it_day)
    {
        const auto total_start = std::chrono::high_resolution_clock::now();

        skip_weekend(it_day);

        fmt::print(
            fmt::emphasis::italic, "Current day {} - {:n} days to go\n", boost::gregorian::to_simple_string(*it_day), cfg.days - days);

        utils::timespec ts{utils::extract_seconds(*it_day), std::chrono::nanoseconds{0}};

        ts += std::chrono::hours{9};

        // increment is 8 hours divided by iterations

        const auto start = std::chrono::high_resolution_clock::now();

        const std::chrono::nanoseconds increment{
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::hours{8}).count() / cfg.iterations};

        gen.randomize_max_volume();

        const std::uint64_t this_day_iterations = gen.iterations_count(cfg.iterations);

        for (std::uint64_t i = 0; i < this_day_iterations; ++i, ts += increment)
        {
            if (should_generate_error(cfg, days, i))
            {
                fmt::print(fmt::fg(fmt::color::red), "What are you doing, Dave?\n");
                generate_error_transaction(b, ts, gen);
            }
            else
            {
                generate_transaction(b, ts, gen);
            }
        }

        const auto stop = std::chrono::high_resolution_clock::now();

        fmt::print("Generated {:n} transactions in {}\n", this_day_iterations,
            std::chrono::duration_cast<std::chrono::milliseconds>(stop - start));

        push_to_db(b);

        const auto total_end = std::chrono::high_resolution_clock::now();

        fmt::print(fmt::emphasis::italic | fmt::fg(fmt::color::gray), "Estimated {} to go...\n\n",
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds{
                std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count() * (cfg.days - days)}));
    }

    qdb_release(h, b);

    const auto global_end = std::chrono::high_resolution_clock::now();

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::green), "Insertion took {}\n",
        std::chrono::duration_cast<std::chrono::seconds>(global_end - global_start));
}

int main(int argc, char ** argv)
{
    try
    {
        fmt::print(fmt::fg(fmt::color::blue), "quasardb transaction log generator\n");

        const config cfg = parse_config(argc, argv);

        qdb::handle h;

        qdb_error_t err = h.connect(cfg.qdb_url.c_str());
        throw_on_failure(err, "connection error");

        create_agents(h);
        create_counterparties(h);
        create_securities(h);
        create_table(h);
        generate_transaction_log(h, cfg);

        h.close();

        return EXIT_SUCCESS;
    }
    catch (const std::exception & e)
    {
        fmt::print(fmt::fg(fmt::color::red), "exception caught: {}", e.what());
        return EXIT_FAILURE;
    }
}

#include "dow_jones.hpp"
#include "products.hpp"
#include "trader.hpp"
#include <qdb/client.hpp>
#include <qdb/integer.h>
#include <qdb/tag.h>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/program_options.hpp>
#include <fmt/format.h>
#include <iostream>
#include <random>
#include <thread>

static constexpr std::uint64_t min_iterations = 1'000;

struct config
{
    std::string qdb_url;
    bool fast;
    std::uint64_t iterations;
};

static void throw_on_failure(qdb_error_t err, const char * msg)
{
    if (QDB_FAILURE(err))
    {
        fmt::print("Error: {} ({})\n", qdb_error(err), err);
        throw std::runtime_error(msg);
    }
}

class trading
{
public:
    explicit trading(qdb_handle_t h) : _generator{_random()}, _handle{h}, _wait_interval{10, 300}, _volume_distribution{100, 1'000}
    {
    }

    template <typename Trader>
    void operator()(Trader & trd) const
    {
        trade t;
        std::vector<quote> quotes;

        std::tie(t, quotes) = trd(_volume_distribution(_generator));

        qdb_error_t err = insert_into_qdb(_handle, t);
        throw_on_failure(err, "cannot insert trade");

        for (const quote & q : quotes)
        {
            err = insert_into_qdb(_handle, q);
            throw_on_failure(err, "cannot insert quote");
        }

        fmt::print(
            "Trader {:5}: Broker {:5} Product {:4} Volume {:5} Value {:10}\n", t.trader, t.counterparty, t.product, t.volume, t.value);
        std::this_thread::sleep_for(std::chrono::milliseconds(_wait_interval(_generator)));
    }

private:
    std::random_device _random;
    mutable std::minstd_rand _generator;

    qdb_handle_t _handle;

    std::uniform_int_distribution<std::uint32_t> _wait_interval;
    std::uniform_int_distribution<std::uint32_t> _volume_distribution;
};

class fast_trading
{
public:
    explicit fast_trading(qdb_handle_t h, std::uint64_t count) : _generator{100.0, 5.0}, _handle{h}, _count{count}
    {
    }

    void operator()()
    {
        fmt::print("generating {:n} points\n", _count);

        quotes_in_cols quotes{"bloom", "AAPL"};

        utils::timespec start_time = utils::timestamp();

        const auto timer_start = std::chrono::high_resolution_clock::now();

        for (std::uint64_t i = 0; i < _count; i += min_iterations)
        {
            start_time = quotes.generate(_generator, start_time, min_iterations);

            qdb_error_t err = insert_into_qdb(_handle, quotes);
            throw_on_failure(err, "cannot insert quotes");
        }

        const auto timer_end = std::chrono::high_resolution_clock::now();

        using double_seconds = std::chrono::duration<double>;

        fmt::print("insertions per second: {:n}\n",
            static_cast<std::uint64_t>(_count / std::chrono::duration_cast<double_seconds>(timer_end - timer_start).count()));
    }

private:
    quote_generator _generator;

    qdb_handle_t _handle;
    const std::uint64_t _count;
};

static config parse_config(int argc, char ** argv)
{
    config cfg;

    boost::program_options::options_description desc{"Allowed options"};
    desc.add_options()                                                                                               //
        ("help", "produce help message")                                                                             //
        ("fast", "fast, silent generator")                                                                           //
        ("url", boost::program_options::value<std::string>(&cfg.qdb_url)->default_value("qdb://127.0.0.1:2836"))     //
        ("iterations", boost::program_options::value<std::uint64_t>(&cfg.iterations)->default_value(min_iterations)) //
        ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    boost::program_options::notify(vm);

    if ((cfg.iterations % min_iterations) != 0)
    {
        std::cerr << "iterations must be multiples of " << min_iterations << std::endl;
        std::exit(1);
    }

    cfg.fast = vm.count("fast") > 0;

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        std::exit(0);
    }

    return cfg;
}

static products make_products()
{
    products prods;

    add_dow_jones(prods);

    return prods;
}

static brokers make_brokers(broker & master_broker)
{
    brokers brks;

    add_broker(brks, "bloom", master_broker);
    add_broker(brks, "tom", master_broker);
    add_broker(brks, "xtnd", master_broker);
    add_broker(brks, "nile", master_broker);

    return brks;
}

struct traders_ts_creator
{
    explicit traders_ts_creator(qdb_handle_t h) : _handle{h}
    {
    }

    template <typename Trader>
    void operator()(Trader & trd) const
    {
        qdb_error_t err = create_trader_ts(_handle, trd.name());
        throw_on_failure(err, "cannot create trader ts");
    }

private:
    qdb_handle_t _handle;
};

void create_products_ts(qdb_handle_t h, const brokers & brks, const products & prods)
{
    for (const auto & prd : prods)
    {
        static const char * tag = "@products";

        qdb_error_t err = qdb_int_put(h, prd.first.c_str(), 0, qdb_never_expires);
        err = qdb_attach_tag(h, prd.first.c_str(), tag);
        throw_on_failure(err, "cannot tag product");

        err = qdb_attach_tag(h, tag, "@tags");
        throw_on_failure(err, "cannot tag tag");
    }

    for (const auto & brk : brks)
    {
        static const char * tag = "@brokers";

        qdb_error_t err = qdb_int_put(h, brk.first.c_str(), 0, qdb_never_expires);
        err = qdb_attach_tag(h, brk.first.c_str(), tag);
        throw_on_failure(err, "cannot tag broker");

        err = qdb_attach_tag(h, tag, "@tags");
        throw_on_failure(err, "cannot tag tag");

        for (const auto & prd : prods)
        {
            qdb_error_t err = create_product_ts(h, brk.first, prd.first);
            throw_on_failure(err, "cannot create product ts");
        }
    }
}

int main(int argc, char ** argv)
{
    try
    {
        fmt::printf("quasardb trading desk simulator\n");

        const config cfg = parse_config(argc, argv);

        products prods = make_products();
        broker master_broker{prods};
        brokers brks = make_brokers(master_broker);

        boost::fusion::vector<trader<greedy>, trader<greedy>, trader<greedy>, trader<cheater>> traders(trader<greedy>{"Bob", brks, prods},
            trader<greedy>{"Alice", brks, prods}, trader<greedy>{"Carry", brks, prods}, trader<cheater>{"Cobra", brks, prods});

        std::random_device rnd;
        std::minstd_rand generator{rnd()};

        qdb::handle h;

        qdb_error_t err = h.connect(cfg.qdb_url.c_str());
        throw_on_failure(err, "connection error");

        create_products_ts(h, brks, prods);

        if (cfg.fast)
        {
            fast_trading{h, cfg.iterations}();
        }
        else
        {
            boost::fusion::for_each(traders, traders_ts_creator{h});

            trading trd{h};

            for (int i = 0; i < cfg.iterations; ++i)
            {
                boost::fusion::for_each(traders, trd);
            }
        }

        h.close();

        return EXIT_SUCCESS;
    }
    catch (const std::exception & e)
    {
        fmt::print("exception caught: {}", e.what());
        return EXIT_FAILURE;
    }
}
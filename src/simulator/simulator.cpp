
#include "dow_jones.hpp"
#include "products.hpp"
#include "trader.hpp"

#include <qdb/client.hpp>

#include <fmt/format.h>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <boost/program_options.hpp>

#include <random>
#include <thread>

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
        if (QDB_FAILURE(err)) throw std::runtime_error("cannot insert trade");

        for (const quote & q : quotes)
        {
            err = insert_into_qdb(_handle, q);
            if (QDB_FAILURE(err)) throw std::runtime_error("cannot insert quote");
        }

        fmt::print("Trader {} : Broker {} Product {} Volume {} Value {}\n", t.trader, t.counterparty, t.product, t.volume, t.value);

        std::this_thread::sleep_for(std::chrono::milliseconds(_wait_interval(_generator)));
    }

private:
    std::random_device _random;
    mutable std::minstd_rand _generator;

    qdb_handle_t _handle;

    std::uniform_int_distribution<std::uint32_t> _wait_interval;
    std::uniform_int_distribution<std::uint32_t> _volume_distribution;
};

struct config
{
    std::string qdb_url;
    int iterations;
};

static config parse_config(int argc, char ** argv)
{
    config cfg;

    boost::program_options::options_description desc{"Allowed options"};
    desc.add_options()("help", "produce help message")(
        "url", boost::program_options::value<std::string>(&cfg.qdb_url)->default_value("qdb://127.0.0.1:2836"))(
        "iterations", boost::program_options::value<int>(&cfg.iterations)->default_value(1000));

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    boost::program_options::notify(vm);

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

int main(int argc, char ** argv)
{
    try
    {
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

        if (QDB_FAILURE(err)) throw std::runtime_error("connection error");

        trading trd{h};

        for (int i = 0; i < cfg.iterations; ++i)
        {
            boost::fusion::for_each(traders, trd);
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
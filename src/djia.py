# pylint: disable=C0111
import argparse
import datetime
import time
import quasardb


def connect(url):
    cluster = quasardb.Cluster(url)
    print "OK> Connected to", url
    return cluster


def get_tagged_elements(cluster):
    elems = {}
    tags = cluster.tag("@tags").get_entries()
    for tag in tags:
        print
        print '{}:'.format(tag[1:])
        # WTF: get_entries returns a tuple!
        elems[tag] = sorted(cluster.tag(tag).get_entries())
        for idx, elem in enumerate(elems[tag]):
            print '{:4}. {}'.format(idx, elem)

    return elems


def compute_index(cluster, index, products, args):
    dow_divisor = 0.14602128057775

    now = datetime.datetime.now(quasardb.tz)
    agg = quasardb.TimeSeries.DoubleAggregations()
    agg.append(quasardb.TimeSeries.Aggregation.last,
               (now - datetime.timedelta(days=1), now))

    if args.show_products:
        print
        print 'Products of {}:'.format(index)

    product_sum = 0
    for prod in products:
        product_ts = cluster.ts(prod)
        col = product_ts.column(
            quasardb.TimeSeries.DoubleColumnInfo('value'))
        res = col.aggregate(agg)
        value = res[0].value

        product_sum = product_sum + value

        if args.show_products:
            print '{:5}: value {}'.format(prod, value)

    return product_sum / dow_divisor


def compute_indexes(cluster, elems, args):
    for index in elems['@indexes']:
        index_value = compute_index(cluster, index, elems['@products'], args)

        if args.show_products:
            print
        print 'Index of {} = {}'.format(index, index_value)


def main():
    parser = argparse.ArgumentParser(description='Compute real-time index')
    parser.add_argument('--interval', type=int, default=1, metavar='SECONDS',
                        help='number of seconds between each refresh')
    parser.add_argument('--hide-products', dest='show_products', action='store_false',
                        help='whether to hide values of each product')
    args = parser.parse_args()

    cluster = connect("qdb://127.0.0.1:2836")
    elems = get_tagged_elements(cluster)

    while True:
        compute_indexes(cluster, elems, args)
        time.sleep(args.interval)


if __name__ == "__main__":
    main()

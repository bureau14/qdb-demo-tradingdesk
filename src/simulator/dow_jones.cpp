#include "dow_jones.hpp"

void add_dow_jones(products & prds)
{
    static const char * index = "Dow Jones Industrial Average";

    add_product(prds, "MMM", 190.0, 1.0, index);
    add_product(prds, "AXP", 80.0, 1.1, index);
    add_product(prds, "AAPL", 139.0, 1.5, index);
    add_product(prds, "BA", 183.0, 0.2, index);
    add_product(prds, "CAT", 95.0, 0.3, index);
    add_product(prds, "CVX", 114.0, 0.8, index);
    add_product(prds, "CSCO", 35.0, 0.2, index);
    add_product(prds, "KO", 43.0, 0.01, index);
    add_product(prds, "DD", 79.7, 0.2, index);
    add_product(prds, "XOM", 83.3, 1.0, index);
    add_product(prds, "GE", 30.0, 0.1, index);
    add_product(prds, "GS", 251.0, 1.0, index);
    add_product(prds, "HD", 147.5, 1.0, index);
    add_product(prds, "IBM", 180.0, 1.5, index);
    add_product(prds, "INTC", 36.0, 0.01, index);
    add_product(prds, "JNJ", 124.0, 1.0, index);
    add_product(prds, "JPM", 92.0, 1.0, index);
    add_product(prds, "MCD", 128.0, 1.5, index);
    add_product(prds, "MRK", 66.0, 1.0, index);
    add_product(prds, "MSFT", 64.0, 1.5, index);
    add_product(prds, "NKE", 57.8, 1.6, index);
    add_product(prds, "PFE", 64.0, 1.0, index);
    add_product(prds, "PG", 91.0, 1.0, index);
    add_product(prds, "TRV", 124.0, 1.5, index);
    add_product(prds, "UNH", 168.0, 2.0, index);
    add_product(prds, "UTX", 112.0, 1.0, index);
    add_product(prds, "VZ", 50.0, 0.5, index);
    add_product(prds, "V", 88.53, 0.1, index);
    add_product(prds, "WMT", 70.76, 0.1, index);
    add_product(prds, "DIS", 110.0, 1.0, index);
}

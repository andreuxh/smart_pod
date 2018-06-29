#include "smart_pod.hh"

SMART_POD(foo)
    SPOD_RW_FIELD(double,   d);
    SPOD_RW_FIELD(int,      i);
    SPOD_RW_FIELD(char[20], s);
SMART_POD_END; 

#include <iostream>

int main()
{
    foo x;
    x.set_d(0.1);
    x.set_i(-2);
    x.set_s("\"bar\",\"baz\"\\\n\"\b\f\r\t\"");

    std::cout << spod::json_dumper(x) << "\n\n";

    std::cout << spod::json_dumper(x.get_d()) << '\n'
              << spod::json_dumper(x.get_i()) << '\n'
              << spod::json_dumper(x.get_s()) << '\n';

    return 0;
};

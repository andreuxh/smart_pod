#include "smart_pod.hh"

SMART_POD(foo)
    SPOD_RW_FIELD(double,   d);
    SPOD_RW_FIELD(int,      i);
    SPOD_RW_FIELD(char[12], s);
SMART_POD_END; 

#include <iostream>

int main()
{
    foo x;
    x.set_d(0.1);
    x.set_i(-2);
    x.set_s("\"bar\",\"baz\"");

    std::cout << spod::json_dumper(x) << std::endl;

    return 0;
};

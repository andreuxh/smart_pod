#include "smart_pod.hh"

SMART_POD(foo)
    SPOD_RW_FIELD(double,   d);
    SPOD_RW_FIELD(int,      i);
    SPOD_RW_FIELD(char[20], s);
SMART_POD_END;

static_assert(std::is_pod<foo>::value, "foo should be POD");

SMART_POD(foo2)
    SPOD_RW_FIELD(foo, f1); 
    SPOD_RW_FIELD(foo, f2);
SMART_POD_END;

static_assert(std::is_pod<foo2>::value, "foo2 should be POD");


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

    foo2 y;
    y.get_f1().set_d(3.14);
    y.get_f1().set_i(42);
    y.get_f1().set_s("bar");
    auto& baz = y.get_f2();
    baz.set_d(2.71828);
    baz.set_i(0);
    baz.set_s("baz");
    std::cout << '\n' << spod::json_dumper(y) << '\n';

    return 0;
};

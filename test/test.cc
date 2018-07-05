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

SMART_POD(point4d)
    SPOD_RW_FIELD(double[4], x);
SMART_POD_END;

static_assert(std::is_pod<point4d>::value, "point4d should be POD");


#include <stdint.h>

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
              << spod::json_dumper(x.get_s()) << "\n\n";

    foo2 y;
    y.get_f1().set_d(3.14);
    y.get_f1().set_i(42);
    y.get_f1().set_s("bar");
    auto& baz = y.get_f2();
    baz.set_d(2.71828);
    baz.set_i(0);
    baz.set_s("baz");
    std::cout << '\n' << spod::json_dumper(y) << "\n\n";

    point4d m;
    m.get_x()[0] = 1.01;
    m.get_x()[1] = 2.02;
    m.get_x()[2] = 3.03;
    m.get_x()[3] = 4.04;
    std::cout << spod::json_dumper(m) << '\n';
    int pair[] = {1, 2};
    std::cout << spod::json_dumper(pair) << "\n\n";

    unsigned const char ubb[] = "bébé";
      signed const char sbb[] = "bébé";
    std::cout << spod::json_dumper(ubb) << '\n';
    std::cout << spod::json_dumper(sbb) << "\n\n";

    uint8_t u8[] = {0, 255, 1, 254};
    int8_t  i8[] = {0, 127, -128, 1, 126, -127, -1};
    std::cout << spod::json_dumper(u8) << '\n';
    std::cout << spod::json_dumper(i8) << "\n\n";

    bool b[] = {false, true};
    std::cout << spod::json_dumper(b) << "\n\n";

    int i = 33;
    int *p = &i;
    std::cout << spod::json_dumper(p) << '\n';
    p = nullptr;
    std::cout << spod::json_dumper(p) << "\n\n";

    return 0;
};

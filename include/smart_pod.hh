#ifndef SMART_POD_HH
#define SMART_POD_HH


#include <string.h>
#include <type_traits>


#if __COUNTER__ + 1 == __COUNTER__
#define SPOD_COUNTER __COUNTER__
#else
#warning "No working __COUNTER__"
#define SPOD_COUNTER __LINE__
#endif


namespace spod {


template <int i> struct index {};

template <typename T, typename U>
class assign_helper
{
public:
    static void assign(T& x, const U& value)
    {
        x = value;
    }
};

template <typename T, typename U>
void assign(T& x, const U& value)
{
    assign_helper<T, U>::assign(x, value);
};

template <size_t n>
class assign_helper<char[n], const char*>
{
public:
    static void assign(char x[n], const char *value)
    {
        strncpy(x, value, n);
    }
};

template <size_t n, size_t p>
class assign_helper<char[n], char[p]>
{
public:
    static void assign(char x[n], const char value[p])
    {
        strncpy(x, value, n);
    }
};


#define SMART_POD(MyPOD) \
    class MyPOD \
    { \
    public: \
        static constexpr const char *name() { return #MyPOD; } \
    private: \
        enum { SPOD_BeginCount = SPOD_COUNTER + 1 };

#define SPOD_RO_FIELD(Type, Name) \
    private: \
        enum { i_##Name = SPOD_COUNTER - SPOD_BeginCount }; \
        using t_##Name = Type; \
        t_##Name m_##Name; \
    public: \
        static constexpr const char *type_at(spod::index<i_##Name>) \
            { return #Type; } \
        static constexpr const char *name_at(spod::index<i_##Name>) \
            { return #Name; } \
        const t_##Name& value_at(spod::index<i_##Name>) const \
            { return m_##Name; } \
    public: \
        const t_##Name& get_##Name() const { return m_##Name; } \
    private:

#define SPOD_RW_FIELD(Type, Name) \
        SPOD_RO_FIELD(Type, Name) \
    public: \
        template <typename T> \
        void set_##Name(const T& value) \
        { \
            spod::assign(m_##Name, value); \
        } \
    private:

#define SMART_POD_END \
    public: \
        enum { end_index = SPOD_COUNTER - SPOD_BeginCount }; \
    }


namespace dumper {
    template <typename T> class json;
    template <typename T> class json_number;
}

template <typename T>
typename std::enable_if<!std::is_arithmetic<T>::value,
                        dumper::json<T>
                       >::type
json_dumper(const T& x)
{
    return dumper::json<T>(x);
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value,
                        dumper::json_number<T>
                       >::type
json_dumper(T x)
{
    return dumper::json_number<T>(x);
}



namespace dumper {

    template <typename T>
    class json
    {
    public:
        json(const T& pod) : m_pod(pod) {}

        template <typename Os>
        Os& dump(Os& os) const
        {
            return this->dump_to(os, index<T::end_index>());
        }

    private:
        template <typename Os>
        Os& dump_to(Os& os, index<-1>) const
        {
            return os;
        }

        template <typename Os, int i>
        Os& dump_to(Os& os, index<i>) const
        {
            return this->dump_to(os, index<i - 1>())
                << (i ? ',' : '{')
                << '"' << m_pod.name_at(index<i>()) << "\":"
                << json_dumper(m_pod.value_at(index<i>()));
        }

        template <typename Os>
        Os& dump_to(Os& os, index<T::end_index>) const
        {
            return this->dump_to(os, index<T::end_index - 1>()) << '}';
        }

        const T& m_pod;
    };

    template <typename T>
    class json_number
    {
    public:
        json_number(T num) : m_num(num) {}
        
        template <typename Os>
        Os& dump(Os& os) const
        {
            return os << m_num;
        }

    private:
        T m_num;
    };

    class json_string
    {
    public:
        json_string(const char *buf, size_t size)
          : m_buf(buf), m_size(size)
        {}
        
        template <typename Os>
        Os& dump(Os& os) const
        {
            os << '"';

            auto p = m_buf;
            auto size = m_size;
            for (; size && *p; size--, p++)
            {
                auto c = *p;
                switch (c)
                {
                case '\\':
                case '"':
                    break;
                case '\b':
                    c = 'b';
                    break;
                case '\f':
                    c = 'f';
                    break;
                case '\n':
                    c = 'n';
                    break;
                case '\r':
                    c = 'r';
                    break;
                case '\t':
                    c = 't';
                    break;
                default:
                    os << c;
                    continue;
                }
                os << '\\' << c;
            }

            return os << '"';
        }

    private:
        const char *m_buf;
        size_t m_size;
    };

    template <size_t n>
    class json<char[n]> : public json_string
    {
    public:
        json(const char buf[n])
          : json_string(buf, n)
        {}
    };

} // namespace dumper

} // namespace spod


template <typename Dumper, typename Os>
auto operator<<(Os& os, const Dumper& dumper)
    -> decltype(dumper.dump(os))
{
    return dumper.dump(os);
}


#endif // SMART_POD_HH

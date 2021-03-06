#ifndef SMART_POD_HH
#define SMART_POD_HH

#include <ctype.h>
#include <stdio.h>
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

#define SPOD_FIELD_COMMON_(Type, Name, BitWidth_Suffix) \
    private: \
        enum { i_##Name = SPOD_COUNTER - SPOD_BeginCount }; \
        using t_##Name = Type; \
        t_##Name m_##Name BitWidth_Suffix; \
    public: \
        static constexpr const char *type_at(spod::index<i_##Name>) \
            { return #Type; } \
        static constexpr const char *name_at(spod::index<i_##Name>) \
            { return #Name; } \
    private:

#define SPOD_RO_FIELD(Type, Name) \
        SPOD_FIELD_COMMON_(Type, Name,) \
    public: \
        const t_##Name& value_at(spod::index<i_##Name>) const \
            { return m_##Name; } \
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
        t_##Name& get_##Name() { return m_##Name; } \
    private:

#define SPOD_RO_BITFIELD(Type, Name, BitWidth) \
        SPOD_FIELD_COMMON_(Type, Name, :BitWidth) \
    public: \
        Type value_at(spod::index<i_##Name>) const \
            { return m_##Name; } \
        Type get_##Name() const { return m_##Name; } \
    private:

#define SPOD_RW_BITFIELD(Type, Name, BitWidth) \
        SPOD_RO_BITFIELD(Type, Name, BitWidth) \
    public: \
        void set_##Name(Type value) \
            { m_##Name =  value; } \
    private:

#define SPOD_DUMPER(MakeDumper) \
    public: \
        template <typename Os> \
        Os& dump(Os& os) const \
        { \
            return os << MakeDumper(*this); \
        } \
    private:

#define SMART_POD_END \
    public: \
        enum { end_index = SPOD_COUNTER - SPOD_BeginCount }; \
    }


namespace dumper {
    template <typename T> class json;
    template <typename T> class json_number;
    class json_boolean;
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

dumper::json_boolean json_dumper(bool x);
dumper::json_number<unsigned int> json_dumper(unsigned char x);
dumper::json_number<signed int> json_dumper(signed char x);


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
    class json_array
    {
    public:
        json_array(const T *ary, size_t size)
          : m_ary(ary), m_size(size)
        {}
        
        template <typename Os>
        Os& dump(Os& os) const
        {
            for (size_t i = 0; i < m_size; i++)
            {
                os << (i ? ',' : '[') << json_dumper(m_ary[i]);
            }
            return os << ']';
        }

    private:
        const T *m_ary;
        size_t m_size;
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
                    if (isascii(c) && iscntrl(c))
                    {
                        char buf[7];
                        sprintf(buf, "\\u00%02X", c);
                        os << buf;
                    }
                    else
                    {
                        os << c;
                    }
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

    class json_boolean
    {
    public:
        json_boolean(bool x)
          : m_val(x)
        {}

        template <typename Os>
        Os& dump(Os& os) const
        {
            return os << (m_val ? "true" : "false");
        }

    private:
        bool m_val;
    };

    template <typename T>
    class json_optional
    {
    public:
        json_optional(const T *p)
          : m_ptr(p)
        {}

        template <typename Os>
        Os& dump(Os& os) const
        {
            if (m_ptr)
            {
                return os << json_dumper(*m_ptr);
            }
            else
            {
                return os << "null";
            }
        }

    private:
        const T *m_ptr;
    };

    template <typename T, size_t n>
    class json<T[n]> : public json_array<T>
    {
    public:
        json(const T ary[n])
          : json_array<T>(ary, n)
        {}
    };

    template <size_t n>
    class json<char[n]> : public json_string
    {
    public:
        json(const char buf[n])
          : json_string(buf, n)
        {}
    };

    template <typename T>
    class json<T*> : public json_optional<T>
    {
    public:
        json(const T *x)
          : json_optional<T>(x)
        {}
    };

} // namespace dumper


dumper::json_boolean json_dumper(bool x)
{
    return dumper::json_boolean(x);
}

dumper::json_number<unsigned int> json_dumper(unsigned char x)
{
    return json_dumper((unsigned int)x);
}

dumper::json_number<signed int> json_dumper(signed char x)
{
    return json_dumper((signed int)x);
}


} // namespace spod


template <typename Dumper, typename Os>
auto operator<<(Os& os, const Dumper& dumper)
    -> decltype(dumper.dump(os))
{
    return dumper.dump(os);
}


#endif // SMART_POD_HH

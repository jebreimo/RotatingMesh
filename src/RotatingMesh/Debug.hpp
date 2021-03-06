/* JEBDebug: C++ macros and functions for debugging and profiling
 * Copyright 2014 Jan Erik Breimo
 * All rights reserved.
 *
 * This file is distributed under the BSD License.
 * License text is included with the source distribution.
 */
#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>

namespace JEBDebug
{
    class Stream
    {
    public:
        std::ostream& operator()()
        {
            if (!m_Stream)
                m_Stream = &std::clog;
            return *m_Stream;
        }

        void setStream(std::ostream& stream)
        {
            m_Stream = &stream;
        }

    private:
        std::ostream* m_Stream = nullptr;
    };

    static Stream STREAM;
}

#ifdef _MSC_VER
    #define _JEBDEBUG_STREAM_LOCATION() \
        __FILE__ "(" << __LINE__ << "): " << __FUNCSIG__
#else
    #define _JEBDEBUG_STREAM_LOCATION() \
        __FILE__ ":" << __LINE__ << ": " << __PRETTY_FUNCTION__
#endif

#define JEB_CHECKPOINT() \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() << std::endl; \
    } while (false)

#define JEB_MESSAGE(msg) \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() \
            << ":\n\t" << msg << std::endl; \
    } while (false)

// This "recursive" implementation of JEB_SHOW is inspired by the following
// reply on stackoverflow: https://stackoverflow.com/a/5048661
#define _JEBDEBUG_NUM_ARGS2(X, X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, N, ...) N

#define _JEBDEBUG_NUM_ARGS(...) _JEBDEBUG_NUM_ARGS2(0, __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define _JEBDEBUG_SHOW_1(var, ...) \
    << "\n\t" #var " = " << (var)

#define _JEBDEBUG_SHOW_2(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_1(__VA_ARGS__)

#define _JEBDEBUG_SHOW_3(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_2(__VA_ARGS__)

#define _JEBDEBUG_SHOW_4(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_3(__VA_ARGS__)

#define _JEBDEBUG_SHOW_5(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_4(__VA_ARGS__)

#define _JEBDEBUG_SHOW_6(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_5(__VA_ARGS__)

#define _JEBDEBUG_SHOW_7(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_6(__VA_ARGS__)

#define _JEBDEBUG_SHOW_8(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_7(__VA_ARGS__)

#define _JEBDEBUG_SHOW_9(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_8(__VA_ARGS__)

#define _JEBDEBUG_SHOW_10(var, ...) \
    << "\n\t" #var " = " << (var) \
    _JEBDEBUG_SHOW_9(__VA_ARGS__)

#define _JEBDEBUG_SHOW_N_1(n, ...) \
    _JEBDEBUG_SHOW_##n(__VA_ARGS__)

#define _JEBDEBUG_SHOW_N(n, ...) \
    _JEBDEBUG_SHOW_N_1(n, __VA_ARGS__)

#define JEB_SHOW(...) \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() << ":" \
            _JEBDEBUG_SHOW_N(_JEBDEBUG_NUM_ARGS(__VA_ARGS__), __VA_ARGS__) \
            << std::endl; \
    } while (false)

#define _JEBDEBUG_UNIQUE_NAME_EXPANDER2(name, lineno) name##_##lineno
#define _JEBDEBUG_UNIQUE_NAME_EXPANDER1(name, lineno) \
    _JEBDEBUG_UNIQUE_NAME_EXPANDER2(name, lineno)
#define _JEBDEBUG_UNIQUE_NAME(name) \
    _JEBDEBUG_UNIQUE_NAME_EXPANDER1(name, __LINE__)

#define _JEBDEBUG_AS_STRING_TRICK(a) #a
#define _JEBDEBUG_AS_STRING(a) _JEBDEBUG_AS_STRING_TRICK(a)

#ifdef _MSC_VER
    #define _JEBDEBUG_CONTEXT() \
        (std::string(__FILE__ "(" _JEBDEBUG_AS_STRING(__LINE__) "): ") + \
        __FUNCSIG__)
#else
    #define _JEBDEBUG_CONTEXT() \
        (std::string(__FILE__ ":" _JEBDEBUG_AS_STRING(__LINE__) ": ") + \
        __PRETTY_FUNCTION__)
#endif

namespace JEBDebug
{
    using std::chrono::high_resolution_clock;

    class CpuTimer
    {
    public:
        void start()
        {
            m_StartTime = high_resolution_clock::now();
            m_IsStopped = false;
        }

        void stop()
        {
            auto endTime = high_resolution_clock::now();
            m_AccumulatedTime += endTime - m_StartTime;
            m_IsStopped = true;
        }

        double seconds() const
        {
            using namespace std::chrono;
            auto tmp = m_AccumulatedTime;
            if (!stopped())
            {
                auto endTime = high_resolution_clock::now();
                tmp += endTime - m_StartTime;
            }
            return duration<double>(tmp).count();
        }

        bool stopped() const
        {
            return m_IsStopped;
        }

    private:
        high_resolution_clock::time_point m_StartTime = {};
        high_resolution_clock::duration m_AccumulatedTime = {};
        bool m_IsStopped = true;
    };

    template <typename String>
    class ScopedTimerImpl
    {
    public:
        typedef typename String::value_type CharT;
        typedef std::basic_ostream<CharT, std::char_traits<CharT>> Stream;

        ScopedTimerImpl(const String& label, Stream& stream)
            : m_Label(label),
              m_Stream(stream)
        {
            m_Timer.start();
        }

        ~ScopedTimerImpl()
        {
            m_Timer.stop();
            m_Stream << m_Label << m_Timer << "\n";
        }

    private:
        CpuTimer m_Timer;
        String m_Label;
        Stream& m_Stream;
    };

    typedef ScopedTimerImpl<std::string> ScopedTimer;

    template <typename Char, typename Traits>
    std::basic_ostream<Char, Traits>& operator<<(
        std::basic_ostream<Char, Traits>& os,
        const CpuTimer& stopwatch)
    {
        return os << stopwatch.seconds();
    }
}

#define JEB_TIMEIT() \
    ::JEBDebug::ScopedTimer _JEBDEBUG_UNIQUE_NAME(JEB_ScopedTimer) \
        (_JEBDEBUG_CONTEXT() + ":\n\telapsed time = ", ::JEBDebug::STREAM())

namespace JEBDebug { namespace internal
{
    template <typename It>
    void write(std::ostream& os, It begin, It end)
    {
        if (begin != end)
            os << *begin++;
        for (; begin != end; begin++)
            os << ", " << *begin;
    }

    template <typename Container>
    void writeContainer(std::ostream& os, const Container& c)
    {
        write(os, std::begin(c), std::end(c));
    }

    template <typename It>
    void writePretty(std::ostream& os, It begin, It end)
    {
        size_t i = 0;
        for (; begin != end; begin++)
            os << std::setw(6) << i++ << ": " << *begin << "\n\t";
    }

    template <typename Container>
    void writeContainerPretty(std::ostream& os, const Container& c)
    {
        writePretty(os, std::begin(c), std::end(c));
    }
}}

#define JEB_SHOW_RANGE_FLAT(begin, end) \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() \
            << ":\n\t" #begin " ... " #end " = ["; \
        ::JEBDebug::internal::write(::JEBDebug::STREAM(), (begin), (end)); \
        ::JEBDebug::STREAM() << "]" << std::endl; \
    } while (false)

#define JEB_SHOW_CONTAINER_FLAT(c) \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() \
            << ":\n\t" #c " = ["; \
        ::JEBDebug::internal::writeContainer(::JEBDebug::STREAM(), (c)); \
        ::JEBDebug::STREAM() << "]" << std::endl; \
    } while (false)

#define JEB_SHOW_RANGE(begin, end) \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() \
                    << ":\n\t" #begin " ... " #end " = [\n\t"; \
        ::JEBDebug::internal::writePretty(::JEBDebug::STREAM(), (begin), (end)); \
        ::JEBDebug::STREAM() << "]" << std::endl; \
    } while (false)

#define JEB_SHOW_CONTAINER(c) \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() \
            << ":\n\t" #c " = [\n\t"; \
        ::JEBDebug::internal::writeContainerPretty(::JEBDebug::STREAM(), (c)); \
        ::JEBDebug::STREAM() << "]" << std::endl; \
    } while (false)

namespace JEBDebug
{
    namespace internal
    {
        inline size_t printHexNumbers(std::ostream& stream,
                                      const void* data, size_t numBytes,
                                      size_t numNumbers)
        {
            auto flags = stream.setf(std::ios::hex, std::ios::basefield);
            auto fill = stream.fill('0');
            size_t i = 0;
            auto n = std::min(numBytes, numNumbers);
            const auto* cdata = static_cast<const unsigned char*>(data);
            for (; i < n; ++i)
                stream << ' ' << std::setw(2) << unsigned(cdata[i]);
            for (; i < numNumbers; ++i)
                stream << "   ";
            stream.fill(fill);
            stream.flags(flags);
            return n;
        }

        inline size_t printCharacters(std::ostream& stream,
                                      const void* data, size_t numBytes,
                                      size_t numChars)
        {
            size_t i = 0;
            auto n = std::min(numBytes, numChars);
            const auto* cdata = static_cast<const unsigned char*>(data);
            for (; i < n; ++i)
            {
                if (32 <= cdata[i] && cdata[i] < 127)
                    stream.put(char(cdata[i]));
                else
                    stream.put('.');
            }
            for (; i < numChars; ++i)
                stream.put(' ');
            return n;
        }
    }

    inline void hexdump(std::ostream& stream, const void* data, size_t size,
                        size_t columns = 16)
    {
        auto flags = stream.setf(std::ios::hex, std::ios::basefield);
        auto fill = stream.fill('0');
        const auto* cdata = static_cast<const unsigned char*>(data);
        const auto* end = cdata + size;
        auto digits = [](size_t n)
        {
            size_t i = 1;
            while (n >>= 4u)
                ++i;
            return i;
        }(size);
        while (cdata != end)
        {
            stream << std::setw(digits) << (intptr_t(cdata) - intptr_t(data));
            const auto* tmpCdata = cdata;
            for (int i = 0; i < 2; ++i)
            {
                stream.put(' ');
                tmpCdata += internal::printHexNumbers(stream, tmpCdata,
                                                      end - tmpCdata, columns / 2);
            }
            stream << "  ";
            cdata += internal::printCharacters(stream, cdata,
                                               end - cdata, columns);
            stream.put('\n');
        }
        stream.fill(fill);
        stream.flags(flags);
    }

    template <typename T>
    void hexdump(std::ostream& stream, const T& value)
    {
        using std::data, std::size;
        hexdump(stream, data(value), size(value) * sizeof(decltype(*data(value))));
    }
}

/**
 * @brief Display a nice hexdump of the input variable.
 *
 * The arguments can be
 * - a single variable that supports data(v) and size(v), for instance a
 *   std::vector.
 * - two values, data and size, where data is a pointer to the data to be
 *   displayed, and size is the number of bytes to display.
 */
#define JEB_HEXDUMP(...) \
    do { \
        ::JEBDebug::STREAM() << _JEBDEBUG_STREAM_LOCATION() \
                    << ":\n" #__VA_ARGS__ ":\n"; \
        ::JEBDebug::hexdump(::JEBDebug::STREAM(), __VA_ARGS__); \
        ::JEBDebug::STREAM() << "]" << std::endl; \
    } while (false)

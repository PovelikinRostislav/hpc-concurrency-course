#ifndef COMMON__H_
#define COMMON__H_

#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#include <ostream>
#include <sstream>

// The COMPILE_ASSERT macro can be used to verify that a compile time
// expression is true, for example:
//
//   COMPILE_ASSERT(sizeof(T) == sizeof(U), SizesAreDifferent);
//
// The second argument to the macro is the name which, if expression is false,
// will show up somewhere in the error message (hopefully).
// The reason we use the helper template is to force the expression to be a
// compile-time constant, otherwise compilers with C99 support will silently
// compile arrays of sizes unknown until run time.
//
template <bool> struct CompileAssertHelper {};
#define COMPILE_ASSERT(expression, message) \
    typedef CompileAssertHelper<(bool(expression))> message[(expression) ? 1 : -1]

// The DECLARE_NON_COPYABLE macro can be used for declaring classes non-copyable
// by means of a copy constructor or an assignment operator. 
// To be effective, the declaration must be placed in the private section of the class.
// It is a good practice to make it the last line of the class (assuming
// private: section is at the end), or the very first line, before any access
// declarations.
// Example:
//   class Foo {
//       DECLARE_NON_COPYABLE(Foo);
//       public:
//       Foo();
//       ...
//   };
//   
//   class Bar {
//       public:
//       Bar();
//       ...
//       private:
//       ...
//       DECLARE_NON_COPYABLE(Bar);
//   };
//   
//   class Fubar {
//       public:
//       DECLARE_NON_COPYABLE(Fubar);  // THIS WILL NOT MAKE IT NON-COPYABLE!
//       Fubar();
//       ...
//   };
//   
//   
// This will be so much easier in C++11.
/*
#define DECLARE_NON_COPYABLE(T)  \
    T(const T&) = delete;        \
    void operator=(const T&) = delete
*/
#define DECLARE_NON_COPYABLE(T) \
    T(const T&);                \
    void operator=(const T&)

// The arraysize(arr) macro returns the number of elements in an array.  The
// expression arraysize(arr) is a compile-time constant of type size_t, so it
// can be used to declare sizes of other arrays.
//
// arraysize() cannot be used on pointers, it will not compile.  However, prior
// to C++11, arraysize() does not accept arrays of types defined inside a
// function, or anonymous types.  For these types you have to use the less safe
// ARRAYSIZE() macro below, which can be accidentally used on some pointers
// instead of arrays.

// This template function declaration is used in defining arraysize.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N> char (&ArraySizeHelper(T (&array)[N]))[N];
template <typename T, size_t N> char (&ArraySizeHelper(const T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

// The ARRAYSIZE(arr) performs essentially the same calculation as arraysize,
// but can be used on anonymous types or types defined inside functions.  The
// expression ARRAYSIZE(arr) is a compile-time constant of type size_t.  It's
// less safe than arraysize as it accepts some (although not all) pointers:
// pointers, namely where the pointer size is divisible by the pointee size. On
// a 64-bit system, pointers are 8 bytes, so pointers to 1-byte, 2-byte,
// 4-byte, and 8-byte types are potentially dangerous.
//
// ARRAYSIZE(arr) works by inspecting sizeof(arr) and sizeof(arr[0]).  If the
// former is divisible by the latter, perhaps arr is indeed an array, in which
// case the division result is the number of elements in the array.  Otherwise,
// arr cannot possibly be an array, and we generate a compiler error to prevent
// the code from compiling. The error you get will be "division by zero" or
// something similar.
#define ARRAYSIZE(a) ((sizeof(a) / sizeof(a[0])) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

// Macros to concatenate strings or convert non-string names (such as __LINE__)
// to strings.  Since the preprocessor does not expand macro arguments
// prepended by # or ##, two levels of indirection are required.
#define CPP_CONCAT2(x, y) x ## y
#define CPP_CONCAT(x, y) CPP_CONCAT2(x, y)
#define CPP_STRING2(x) # x
#define CPP_STRING(x) CPP_STRING2(x)

// Macros to generate a unique name by appending the line number to the given
// prefix.
#define UNIQUE_NAME_L(x) CPP_CONCAT(x, __LINE__)

// Current line as a string.
#define LINE_STRING CPP_STRING(__LINE__)

// Current filename and line as a string.
#define FILE_LINE_STRING __FILE__ ":" CPP_STRING(__LINE__)

// C++ has octal and hexadecimal numbers but no binary numbers.
// GCC has a 0bxxxx extension, but it's not portable.
// The BINARY() macro below allows binary numbers to be declared as
// BINARY(xxxx) where x can be 0 or 1 and the maximum length of the number is
// 22 bits (limited by the longest octal number 01111..111 that fits into an
// unsigned long).
// BINARY() is a compile-time constant and can be used anywhere a compile-time
// integer constant is allowed.
// Using digits other than 0 and 1 is a compile-time error.
template <unsigned long N> struct BinGuard; // Disallow digits except 0 and 1
template <> struct BinGuard<0> {
    enum { V = 0 };
};
template <> struct BinGuard<1> {
    enum { V = 1 };
};
template <unsigned long N> struct BinHelper {
    enum { V = BinGuard<N % 8>::V + (BinHelper<N / 8>::V << 1) };
};
template <> struct BinHelper<0> {
    enum { V = 0 };
};
#define BINARY(N) BinHelper<0##N##UL>::V

// Logical expressions with branch prediction.
// On supported compilers, these expressions will generate the code optimized
// for the likely branch. 
// Note that this should not be used as an optimization of somewhat likely
// branches: the programmers are notoriously bad at predicting which choice is
// more likely. The intended use the code handling exceptional conditions, such
// as asserts. The prediction makes this code more efficient, since the cost of
// a correctly predicted branch is usually negligible.
// Examples:
//   if (PREDICT_FALSE(p == NULL)) abort();
//   if (PREDICT_TRUE(n > 0)) Foo(n); else Error();
#ifdef __GNUC__
#define PREDICT_TRUE(expr) __builtin_expect(static_cast<bool>(expr), true)
#define PREDICT_FALSE(expr) __builtin_expect(static_cast<bool>(expr), false)
#else // !__GNUC__
#define PREDICT_TRUE(expr) (expr)
#define PREDICT_FALSE(expr) (expr)
#endif // __GNUC__ 

// Simple logging macros for debug and diagnostics.
// The basic syntax of any logging statement is:
//   LOG << term1 << term2 << ...
// Each line contains the file name and line number where the logging originated from.
// Each line is terminated with a new line so it is not necessary to write endl.
//
// In addition to the simple logging, there are CHECK macros which check the
// specified condition. If the condition is violated, the program is terminated
// and a detailed error message is logged.

// The basic logger. A temporary instance of this class is created for every
// log line.  The logger ensures thread safery of logging as well as performs
// the necessary actions at the end of the log line.
class Logger
{
    public:
    // Logging options.
    enum Options {
        LOG_OPTIONS_DEFAULT = 0x0000,
        LOG_OPTIONS_ERROR   = 0x0001,
        LOG_OPTIONS_FATAL   = 0x0002
    };
    // Constructor usually creates a temporary instance of the Logger.
    // It locks I/O mutex for the entire lifetime of the Logger, to prevent
    // logging from different threads from interleaving.
    Logger(unsigned int options = LOG_OPTIONS_DEFAULT);

    // Destructor flushes the stream and unlocks I/O mutex.
    ~Logger();

    // Inserter allows use of the Logger as if it was a stream:
    //   Logger() << x << y;
    // Note that "<< endl" is not required since the Logger flushes every line.
    template <typename T> Logger& operator<<(const T& x) {
        if (options_ & LOG_OPTIONS_FATAL) *err_ << x;
        else if (options_ & LOG_OPTIONS_ERROR) *err_ << x;
        else *out_ << x;
        return *this;
    }

    // Provide non-template overloads for some of the built-in types (in
    // particular, the overloads for integers allow logging of enums).
#define LOGGER_INSERTER(T) \
    Logger& operator<<(T x) { \
        if (options_ & LOG_OPTIONS_FATAL) *err_ << x; \
        else if (options_ & LOG_OPTIONS_ERROR) *err_ << x; \
        else *out_ << x; \
        return *this; \
    }
    LOGGER_INSERTER(bool)
    LOGGER_INSERTER(char)
    LOGGER_INSERTER(unsigned char)
    LOGGER_INSERTER(signed char)
    LOGGER_INSERTER(short)
    LOGGER_INSERTER(unsigned short)
    LOGGER_INSERTER(int)
    LOGGER_INSERTER(unsigned int)
    LOGGER_INSERTER(long)
    LOGGER_INSERTER(unsigned long)
#undef LOGGER_INSERTER

    // Default configuration uses cout for non-fatal messages and cerr for
    // fatal ones. Configuration can be changed at any time using the Options
    // object and Configure() method:
    //   SetMutex()       : change the mutex used for locking I/O in
    //                      multi-threaded systems.
    //   SetStream()      : change the stream for all log messages except error
    //                      messages
    //   SetErrorStream() : change the stream for all error messages, including fatal
    //                      error messages
    class Config {
        public:
        Config() : mutex_(NULL), out_(NULL), err_(NULL) {}
        Config& SetMutex(pthread_mutex_t* mutex) { mutex_ = mutex; return *this; }
        Config& SetStream(std::ostream* out) { out_ = out; return *this; }
        Config& SetErrorStream(std::ostream* err) { err_ = err; return *this; }
        private:
        friend class Logger;
        pthread_mutex_t* mutex_;
        std::ostream* out_;
        std::ostream* err_;
    };
    static void Configure(const Config& config);

    private:
    unsigned int options_;
    static pthread_mutex_t* mutex_;
    static std::ostream* default_stream_;
    static std::ostream* default_error_stream_;
    std::ostream* out_;
    std::ostream* err_;
};

// Little helper to enable code like "cond ? (void)0 : Logger()".
// We need an operator with priority below << but above ?:.
// The above code now becomes "cond ? (void)0 : Logger_Voidify() & Logger()".
struct Logger_Voidify {
    void operator&(Logger&) {}
};

// The most basic logging macro.
#define LOG_MESSAGE \
    Logger() << "\n[" << __FILE__ << ":" << __LINE__ << "] "

// Logging macro for errors.
#define LOG_ERROR_NONFATAL \
    Logger(Logger::LOG_OPTIONS_ERROR) << "\n[" << __FILE__ << ":" << __LINE__ << "] ERROR: "

// Logging macro for fatal errors.
// The program will terminate after the line is logged.
// The "quiet" version of this macro is used in checks which print their own
// message instead of "FATAL ERROR".
#define LOG_FATAL_QUIET \
    Logger(Logger::LOG_OPTIONS_FATAL) << "\n[" << __FILE__ << ":" << __LINE__ << "] "
#define LOG_FATAL LOG_FATAL_QUIET << " FATAL ERROR: "

// Helper macro to simplify logging variables:
//   int x = 42, a = 1, b = 2;
//   LOG << LOG_VAL(x) << LOG_VAL(a + b)
// will log
//   ... x=42 a + b=3
#define LOG_VAL(val) #val << "=" << (val) << " "

// All CHECK macros log a fatal error if their conditions are not satisfied.
//
// Assert that the condition is true. The condition expression must be convertible to bool.
#define CHECK(cond) \
    PREDICT_FALSE(cond) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #cond << ". "

// Assert that the condition is false. The condition expression must be convertible to bool.
#define CHECK_FALSE(cond) \
    !PREDICT_TRUE(cond) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #cond << ". "

// Assert that the two values are equal. The values must support comparison for equality.
#define CHECK_EQ(x, y) \
    PREDICT_FALSE((x) == (y)) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " == " << #y << " (" << #x << "=" << (x) << ", " << #y << "=" << (y) << "). "

// Assert that the two values are not equal. The values must support comparison for equality.
#define CHECK_NE(x, y) \
    !PREDICT_TRUE((x) == (y)) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " != " << #y << " (" << #x << "=" << (x) << ", " << #y << "=" << (y) << "). "

// Assert that x is strictly less than y. The values must support comparison.
#define CHECK_LT(x, y) \
    PREDICT_FALSE((x) < (y)) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " < " << #y << " (" << #x << "=" << (x) << ", " << #y << "=" << (y) << "). "

// Assert that x is greater or equal to y. The values must support comparison.
#define CHECK_GE(x, y) \
    !PREDICT_TRUE((x) < (y)) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " >= " << #y << " (" << #x << "=" << (x) << ", " << #y << "=" << (y) << "). "

// Assert that x is strictly greater than y. The values must support comparison.
#define CHECK_GT(x, y) \
    PREDICT_FALSE((y) < (x)) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " > " << #y << " (" << #x << "=" << (x) << ", " << #y << "=" << (y) << "). "

// Assert that x is less or equal to y. The values must support comparison.
#define CHECK_LE(x, y) \
    !PREDICT_TRUE((y) < (x)) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " <= " << #y << " (" << #x << "=" << (x) << ", " << #y << "=" << (y) << "). "

// Assert that the two NULL-terminated character strings are equal.
#define CHECK_STREQ(x, y) \
    PREDICT_FALSE(strcmp(x, y) == 0) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " == " << #y << " (" << #x << "=" << implicit_cast<const char*>(x) << ", " << #y << "=" << implicit_cast<const char*>(y) << "). "

// Assert that the two NULL-terminated character strings are not equal.
#define CHECK_STRNE(x, y) \
    PREDICT_FALSE(strcmp(x, y) != 0) ? (void)0 : Logger_Voidify() & LOG_FATAL_QUIET << "CHECK FAILED: " << #x << " != " << #y << " (" << #x << "=" << implicit_cast<const char*>(x) << ", " << #y << "=" << implicit_cast<const char*>(y) << "). "

// Identity metafunction.
template <typename T> struct IdentityMeta {
    typedef T type;
};

// implicit_cast is a safe version of static_cast or const_cast for implicit
// conversions.
// Examples:
// - Upcasting in a type hierarchy.
// - Performing arithmetic conversions (int32 to int64, int to double, etc.).
// - Adding const or volatile qualifiers.
//
// IdentityMeta is used to make a non-deduced context, which forces all callers to
// explicitly specify the template argument in all cases.
template<typename T> inline T implicit_cast(typename IdentityMeta<T>::type x) {
    return x;
}

// bit_cast is a template function that implements the equivalent of
// "*reinterpret_cast<Dest*>(&source)" but without the danger of the type
// punning.
//
// This is the classic and wrong thing to do:
//   float f = 1;
//   int i = * reinterpret_cast<int*>(&f);
//
// The address-casting method actually produces undefined behavior according to
// the C++ standard 3.10.15: if an object in memory has one type, and a program
// accesses it with a different type, then the result is [usually] undefined
// behavior (conversions between integers, pointers, and floating types fall
// under "usually"). This is done to allow the allow optimizing compilers to
// assume that expressions with different types refer to different memory (gcc
// really does this in versions above 4.0.1).
//
// This problem cannot be worked around by a different cast, since the problem
// is the type punning, not a particular cast. For example, *(int*)&f and
// *reinterpret_cast<int*>(&f) are equally bad.
//
// This bit_cast<> implementation calls memcpy() which is specified by the
// standard, especially by the example in section 3.9. memcpy() is very fast
// since it is inlined by the compiler.
// 
// Restrictions on casted types:
//   The destination type must be default-constructible.
//      Before you start mucking with the implementation to work around that,
//      consider that the memory for the destination must be properly aligned
//      for the right type (it's possible but not easy, talk to fedorp@ if you
//      really need this).
//   Copy constructors or destructors are not called. If the types are not
//   trivially copyable, non-trivial but bad things will happen.
//
// The helper class uses partial specialization to avoid calling memcpy when
// the two types are the same.
template <typename To, typename From> struct bit_cast_helper {
    static To convert(const From& from) {
        To to;
        ::memcpy(&to, &from, sizeof(to));
        return to;
    }
};
template <typename T> struct bit_cast_helper<T, T> {
    static T convert(const T& from) { return from; }
};
template <typename To, typename From> inline To bit_cast(const From& from) {
    // Compile time assertion: sizeof(To) == sizeof(From)
    typedef char ToAndFromMustHaveSameSize[sizeof(To) == sizeof(From) ? 1 : -1];
    return bit_cast_helper<To, From>::convert(from);
}

// lexical_cast is primarily used for conversions between strings and numbers.
// When casting from numeric types to strings, it creates ASCII representation
// as if the number was written out using the << operator. When casting from
// strings to numbers, it parses the string as if the number was read from it
// using the >> operator.
//
// Examples:
//   lexical_cast<std::string>(42) -> "42"
//   lexical_cast<std::string>(true) -> "true"
//   lexical_cast<bool>("false") -> false
//   lexical_cast<long>("1234") -> 1234L
//
// Note that casting from a C-string works the same way as casting from an
// std::string, but casting to a C string (char*) is not possible.
//
// Bool type is handled somewhat specially: in addition to numeric values, it
// is legal to convert values "true" and "false" to bool. String representation
// of bool is not "0" and "1" but "false" and "true".
//
// Note that lexical_cast does not check for invalid string content. Casting
// strings that do not convert to numbers is undefined.
template <typename To, typename From> struct lexical_cast_helper {
    static To convert(const From& from) {
        To to;
        std::stringstream S;
        S << from;
        S >> to;
        return to;
    }
};
// Special case for casting to the same type.
template <typename T> struct lexical_cast_helper<T, T> {
    static T convert(const T& from) { return from; }
};
// Special case for casting from C string to C++ string.
template <> struct lexical_cast_helper<std::string, char*> {
    static std::string convert(char* from) { return from; }
};
template <> struct lexical_cast_helper<std::string, const char*> {
    static std::string convert(const char* from) { return from; }
};
// Special case for casting between bool and strings.
// Bool converts to "true" or "false".
template <> struct lexical_cast_helper<std::string, bool> {
    static std::string convert(bool from) {
        return from ? "true" : "false";
    }
};
// In addition to numbers, "true" and "false" convert to bool. Empty string
// converts to false.
template <> struct lexical_cast_helper<bool, const char*> {
    static bool convert(const char* from) {
        while (*from == ' ') ++from;
        if (*from == '\0') return false;
        if (strcasecmp(from, "true") == 0) return true;
        if (strcasecmp(from, "false") == 0) return false;
        if (strcasecmp(from, "0") == 0) return false;
        return true;
    }
};
template <> struct lexical_cast_helper<bool, char*> {
    static bool convert(char* from) {
        return lexical_cast_helper<bool, const char*>::convert(from);
    }
};
template <> struct lexical_cast_helper<bool, std::string> {
    static bool convert(const std::string& from) {
        return lexical_cast_helper<bool, const char*>::convert(from.c_str());
    }
};
// The cast template itself.
template <typename To, typename From> inline To lexical_cast(const From& from) {
    return lexical_cast_helper<To, From>::convert(from);
}

// Sleep for the specified number of seconds, possibly fractional.
// If awoken by a signal or spuriously, go to sleep again for the remainder of
// the requested time.
inline void SleepForSeconds(double seconds) {
    struct timespec req, rem;
    req.tv_sec = static_cast<time_t>(seconds);
    req.tv_nsec = static_cast<long>((seconds - static_cast<double>(req.tv_sec))*1e9);
    while (nanosleep(&req, &rem) != 0) {
        if (errno == EINTR) {
            req = rem;
            continue;
        }
        LOG_ERROR_NONFATAL << "Sleep interrupted: " << strerror(errno);
        return;
    }
}

// Sleep for the shorted possible period of time.
// This function should be called in polling loops, such as spinlocks.
inline void NanoSleep() {
    static const struct timespec req = { 0, 1 };
    nanosleep(&req, NULL);
}

// Similar to the above, but targeted for do-while polling loops:
// The argument is returned as the return value, this allows using the
// function inside loop conditions so the thread sleeps only if the loop
// does not terminate:
//   size_t sleep_count = 0;
//   do { ... } while (!success && NanoSleep(true, sleep_count));
inline bool NanoSleep(bool x, size_t& sleep_count) {
    static const struct timespec req = { 0, 1 };
    if (++sleep_count == 8) {
        nanosleep(&req, NULL);
        sleep_count = 0;
    }
    return x;
}

#endif // COMMON_H_

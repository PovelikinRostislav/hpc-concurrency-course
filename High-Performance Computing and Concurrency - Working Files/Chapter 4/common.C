#include <common.h>

#include <pthread.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <ostream>
#include <iostream>

using namespace std;

static bool dump_core_on_abort = true;

// Keep the static initializers together and in the right order.
static pthread_mutex_t default_log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t* Logger::mutex_ = &default_log_mutex;
ostream* Logger::default_stream_ = NULL;
ostream* Logger::default_error_stream_ = NULL;

// Change the current configuration.
void Logger::Configure(const Logger::Config& config) {
    if (config.mutex_) mutex_ = config.mutex_;
    if (config.out_) default_stream_ = config.out_;
    if (config.err_) default_error_stream_ = config.err_;
}

// Constructor usually creates a temporary instance of the Logger.  It locks
// I/O mutex for the entire lifetime of the Logger, to prevent logging from
// different threads from interleaving.
Logger::Logger(unsigned int options) : options_(options) {
    if (default_stream_ == NULL) default_stream_ = &cout;
    if (default_error_stream_ == NULL) default_error_stream_ = &cerr;
    out_ = default_stream_;
    err_ = default_error_stream_;
    pthread_mutex_lock(mutex_);
}

// Destructor flushes the stream and unlocks I/O mutex.
Logger::~Logger() {
    if (options_ & LOG_OPTIONS_FATAL) *err_ << endl;
    else if (options_ & LOG_OPTIONS_ERROR) *err_ << endl;
    else *out_ << endl;
    pthread_mutex_unlock(mutex_);
    // Fatal logs end right here.
    // TODO: add stack trace.
    if (options_ & LOG_OPTIONS_FATAL) {
        // Disable core dump.
        if (!dump_core_on_abort) {
            rlimit nocore = { 0, 0 };
            setrlimit(RLIMIT_CORE, &nocore);
        }
        abort();
    }
}



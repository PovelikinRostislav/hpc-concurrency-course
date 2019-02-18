#ifndef TIMERS_H_
#define TIMERS_H_

// IMPORTANT: all programs using these high-resolution timers must be linked with -lrt.

#include <stdint.h>
#include <sys/times.h>
#include <time.h>

// High-resolution time for measuring elapsed real or CPU time.
// The resolution of this timer on most systems should be in nanosecond range.
// This is the base class for several concrete timers that use specific clocks.
class HighResTimer {
    public:
    // Initialize and start timer.
    HighResTimer(clockid_t clock) :clock_(clock) {
        clock_gettime(clock_, &start_);
    }

    // Start the timer again, return value elapsed since the last start.
    double Reset() {
        struct timespec now;
        clock_gettime(clock_, &now);
        double elapsed = now.tv_sec - start_.tv_sec + 1e-9*(now.tv_nsec - start_.tv_nsec);
        start_ = now;
        return elapsed;
    }

    // Check time elapsed since the timer was started or reset.
    double Time() const {
        struct timespec now;
        clock_gettime(clock_, &now);
        return now.tv_sec - start_.tv_sec + 1e-9*(now.tv_nsec - start_.tv_nsec);
    }

    // Elapsed time in nanoseconds.
    uint64_t TimeNS() const {
        struct timespec now;
        clock_gettime(clock_, &now);
        return static_cast<uint64_t>(1e9*(now.tv_sec - start_.tv_sec) + (now.tv_nsec - start_.tv_nsec));
    }

    // Query timer resolution for a given clock.
    static double Resolution(clockid_t clock) {
        struct timespec res;
        clock_getres(clock, &res);
        return res.tv_sec + 1e-9*res.tv_nsec;
    }

    private:
    const clockid_t clock_;
    struct timespec start_;
};

// High-resolution timer for measuring real time.
class HighResRealTimer : public HighResTimer {
    public:
    HighResRealTimer() : HighResTimer(CLOCK_MONOTONIC) {}
};

// High-resolution timer for measuring per-process CPU time.
class HighResCPUTimer : public HighResTimer {
    public:
    HighResCPUTimer() : HighResTimer(CLOCK_PROCESS_CPUTIME_ID) {}
};

// High-resolution timer for measuring per-thread CPU time.
class HighResThreadTimer : public HighResTimer {
    public:
    HighResThreadTimer() : HighResTimer(CLOCK_THREAD_CPUTIME_ID) {}
};

// Clock (accumulating timer) based on high-resolution timers.
// Time accumulation of this clock is not thread-safe, for thread-safe timers
// see base/atomics/atomic-timers.h.
class HighResClock {
    public:
    // Construct lock of the given type, start accumulation from zero time.
    HighResClock(clockid_t clock) : state_(NEVER_STARTED), clock_(clock) {
        reset();
    }

    // Start timing.
    void start_timer() { state_ = RUNNING; clock_gettime(clock_, &start_); }
 
    // Return cpu time in seconds between the stop time and the last start time.
    // The cpu time returned is the cpu time used in the user space of the calling process. 
    // If stop_timer() is called without a preceding start_timer(), -1 is returned.
    // -1 is returned on error.
    double stop_timer() {
        if (state_ != RUNNING) { state_ = INVALID_STOP; return -1.0; } else state_ = STOPPED;
        struct timespec now;
        if (clock_gettime(clock_, &now) != 0) return -1;
        // Set stop_ = 0 to prevent negative time stamps which can happen if stop is "earlier" than start.
        if ((now.tv_sec < start_.tv_sec) || ((now.tv_sec == start_.tv_sec) && (now.tv_nsec < start_.tv_nsec))) stop_ = 0;
        else stop_ = static_cast<uint64_t>(1e9*(now.tv_sec - start_.tv_sec) + (now.tv_nsec - start_.tv_nsec));
        accumulated_ += stop_;
        return stop_*1e-9;
    }
 
    // Return cpu time in seconds between current time and the last start time.  
    // The cpu time returned is the cpu time used in the user space of the calling process. 
    // If read_timer() is called after a valid stop_timer(), the last stop_timer() value is returned.
    // If read_timer() is called without a preceding start_timer(), -1 is returned.
    // -1 is returned on error.
    double read_timer() const { 
        if (state_ == NEVER_STARTED || state_ == INVALID_STOP) return -1;
        if (state_ == STOPPED) return stop_*1e-9;
        struct timespec now;
        if (clock_gettime(clock_, &now) != 0) return -1;
        return now.tv_sec - start_.tv_sec + 1e-9*(now.tv_nsec - start_.tv_nsec);
    } 
 
    // Return cumulative cpu time in seconds.
    // This is the sum of the (stop_timer() - start_timer()) calls.
    double get_cum_time() const { return accumulated_*1e-9; }
 
    //  Reset the cumulative time to zero.    
    void reset() { accumulated_ = 0; }
 
    // Check clock status.
    bool clock_was_started() { return state_ != NEVER_STARTED && state_ != INVALID_STOP; }
    bool clock_is_running()  { return state_ == RUNNING; }

    // Make this class assignable.
    HighResClock& operator=(const HighResClock& rhs);

    private:
    enum State {
        NEVER_STARTED,
        RUNNING,         // start_timer() called
        STOPPED,         // valid stop_timer() call
        INVALID_STOP     // stop_timer() without start_timer()
    };  
    State               state_;
    const clockid_t     clock_;
    struct timespec     start_;
    uint64_t            stop_; // Times are stored in nanoseconds in a 64-bit integer (enough for 585 years).
    uint64_t            accumulated_;
}; 

#endif // TIMERS_H_

#include <timers.h>

#include <string.h>
#include <sys/times.h>
#include <unistd.h>

// HighResClock

// Assignment is permitted despite the const clock_ member,
HighResClock& HighResClock::operator=(const HighResClock& rhs)
{
    state_ = rhs.state_;
    start_ = rhs.start_;
    stop_ = rhs.stop_;
    accumulated_ = rhs.accumulated_;
    return *this;
}


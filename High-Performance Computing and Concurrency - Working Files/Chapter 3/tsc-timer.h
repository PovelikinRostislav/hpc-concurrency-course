#ifndef TSC_TIMER_H_
#define TSC_TIMER_H_

// Reference: http://download.intel.com/embedded/software/IA/324264.pdf

#include <sched.h>
#include <signal.h>
#include <stdint.h>

// Accurate TSC (Time Stamp Counter) timer.
// This class gives access to the hardware TSC timer. The implementation
// attempts to make the time measurements as accurate as possible.
// There are several problems with the TSC timer:
// 1. RDTSC (the assembler instruction accessing the timer) is not a
//    serializing instruction, so the hardware can rearrange the execution
//    order and move some instructions either outside or inside the measured
//    section.
//    This can be prevented by calling CPUID instruction (serializing) to
//    bracket the timed section of the code. Also, the RDTSCP instruction is
//    half-serializing, it prevents earlier instructions from being executed
//    later than RDTSCP but does not prevent later instructions from being
//    executed earlier. Using this instruction allows us to not use CPUID
//    inside the timed section.
//    The best order of instructions for using the TSC timer is the following:
//      ... earlier instructions ...
//      CPUID - all earlier instructions must finish
//      RDTSC - store TSC value
//      ... timed instructions ...
//      RDTSCP - all timed instructions must finish
//      CPUID - later instructions cannot start until now
//      ... later instructions ...
//    Note that the cost of moving the registers to memory after RDTSC is
//    counted as a part of timed instructions, but the cost of moving the
//    registers to memory after RDTSCP is not counted because the move must
//    necessarily occur after the counter is read.
// 2. The program may be migrated from one CPU to another while the test is
//    running. The TSC timer classes do nothing to prevent this, but the helper
//    class CPU_Limiter can be used to restrict the current thread to the
//    current CPU.
// 3. The clock frequency may be changed by the scaling governor. We do not do
//    anything about this, in general the benchmark should be "primed" by
//    running CPU at full load (usually run few "dummy" passes of the test) for
//    some time before doing the real timed test.
//
// Example:
//   {
//     CPU_Limiter L; // See below
//     AccurateTSCTimer T; T.Start();
//     ... timed code ...
//     unsigned long t = T.Stop();
//     cout << "Code takes " << t << cycles << " or " << t*T.ClockCycle() << "ns" << endl;
//   }
class AccurateTSCTimer
{
    public:
    // Constructor initializes the clock cycle measurement if it was not
    // already done. Note that this initialization will take approximately one
    // real-time second (the first time it happens).
    AccurateTSCTimer();

    // Start the timed section. Unlike the symmetric interface of the
    // HighResTimer class (see timers.h), Start() and Stop() are explicit and
    // different here. This is because they use different serializing
    // instructions to prevent instruction reordering in and out of the timed
    // section.
    void Start() {
        __asm__ __volatile__ (
                "cpuid\n\t"                             // CPUID is a serializing instruction, all preceding instructions must complete
                "rdtsc\n\t"                             // RDTSC is not a serializing instruction
                "mov %%eax, %0\n\t"                     // Save results before CPUID clobbers registers
                "mov %%edx, %1\n\t"
                : "=r"(low1), "=r"(high1)
                : : "%rax", "%rbx", "%rcx", "%rdx");    // Clobbred registers
    }

    // End the timed section and return the time since the last Start() call,
    // in clock cycles.
    unsigned long Stop() {
        __asm__ __volatile__ (
                "rdtscp\n\t"                            // RDTSCP is "half-serializing", all earlier instructions must finish prior to it
                "mov %%eax, %0\n\t"                     // Save results before CPUID clobbers registers
                "mov %%edx, %1\n\t"
                "cpuid\n\t"                             // CPUID is a serializing instruction, no subsequent instructions may run before it
                : "=r"(low2), "=r"(high2)
                : : "%rax", "%rbx", "%rcx", "%rdx");    // Clobbred registers
        uint64_t start = (static_cast<uint64_t>(high1) << 32) | low1;
        uint64_t stop  = (static_cast<uint64_t>(high2) << 32) | low2;
        return stop - start;
    }

    // Similar to Stop() but convert the time to nanoseconds.
    double StopNS() {
        return Stop()*clock_cycle_;
    }

    // Clock cycle duration, in nanoseconds. 
    double ClockCycle() { return clock_cycle_; }

    // Measure the clock cycle (in nanoseconds) and return the new value.
    // The user must call this function before calling StopNS() or ClockCycle().
    // Usually this value is computed once, but it can be recomputed at any
    // time if the user suspects that the clock frequency might have changed.
    static double InitClockCycle();

    private:
    static double clock_cycle_;                         // Nanoseconds
    uint32_t low1, high1, low2, high2;
};

// "Fast" TSC timer.
// This class does not use CPUID instructions to prevent hardware instruction
// reordering. It executes the minimum necessary number of instructions to
// access the TSC counter. 
// Note that in practice the minimum granularity of the FastTSCTimer is not
// always lower than the granularity of the AccurateTSCTimer, i.e. FastTSCTimer
// is not necessarily faster.
class FastTSCTimer
{
    public:
    // Constructor initializes the clock cycle measurement if it was not
    // already done. Note that this initialization will take approximately one
    // real-time second (the first time it happens).
    FastTSCTimer();

    // Start the timed section.
    void Start() {
        __asm__ __volatile__ ("rdtsc" : "=a"(low1), "=d"(high1) : : "%ebx", "%ecx");
    }

    // End the timed section and return the time since the last Start() call,
    // in clock cycles.
    unsigned long Stop() {
        __asm__ __volatile__ ("rdtscp" : "=a"(low2), "=d"(high2) : : "%ebx", "%ecx");
        uint64_t start = (static_cast<uint64_t>(high1) << 32) | low1;
        uint64_t stop  = (static_cast<uint64_t>(high2) << 32) | low2;
        return stop - start;
    }

    // Similar to Stop() but convert the time to nanoseconds.
    double StopNS() {
        return Stop()*clock_cycle_;
    }

    // Clock cycle duration, in nanoseconds.
    double ClockCycle() { return clock_cycle_; }

    // Measure the clock cycle (in nanoseconds) and return the new value.
    static double InitClockCycle();

    private:
    static double clock_cycle_;                         // Nanoseconds
    uint32_t low1, high1, low2, high2;
};

// The CPU_Limiter class restricts the current thread to the current CPU (i.e.
// the CPU it was running on when the class was constructed) for the lifetime
// of the CPU_Limiter object. It also blocks all signals except SIGPROF,
// SIGSEGV, SIGBUS, and SIGTERM.
// 
// Example:
//   {
//     CPU_Limiter L;
//     AccurateTSCTimer T; T.Start();
//     ... timed code - thread is locked to one CPU ...
//     double t = T.StopNS();
//   }
class CPU_Limiter
{
    public:
    CPU_Limiter();
    ~CPU_Limiter();

    private:
    cpu_set_t saved_cpu_;
    sigset_t saved_sigmask_;
};

#endif // TSC_TIMER_H_

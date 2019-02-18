// Test of timer granularity
#include <timers.h>
#include <tsc-timer.h>

#include <unistd.h>

#include <iostream>

using namespace std;

double MeasureStartStop(FastTSCTimer T) {
    double sum = 0;
    static const int N = 1000;
    for (int i = 0; i < N; ++i) {
        T.Start();
        volatile unsigned long t = T.Stop();
        sum += t;
    }
    return sum/N;
}

double MeasureStartStop(AccurateTSCTimer T) {
    double sum = 0;
    static const int N = 1000;
    for (int i = 0; i < N; ++i) {
        T.Start();
        volatile unsigned long t = T.Stop();
        sum += t;
    }
    return sum/N;
}

double MeasureStartStop(const HighResTimer& T) {
    double sum = 0;
    static const int N = 1000;
    for (int i = 0; i < N; ++i) {
        volatile double t1 = T.Time();
        volatile double t2 = T.Time();
        sum += (t2 - t1);
    }
    return sum/N*1e9;
}

int main() {
    CPU_Limiter L;
    // Compute clock cycle.
    double clock_cycle = 0;
    {
        AccurateTSCTimer T1;
        HighResRealTimer T2;
        T1.Start(); double t2a = T2.Time();
        sleep(1);
        unsigned long t = T1.Stop(); double t2b = T2.Time();
        clock_cycle = (t2b - t2a)/(t*1e-9); // nanoseconds
        cout << "TSC: " << t*1e-9 << " G cycles. REAL: " << (t2b - t2a) << " s. Clock cycle: " << clock_cycle << " ns" << endl;
    }
    cout << "TSC time (fast): "  << MeasureStartStop(FastTSCTimer())*clock_cycle        << endl;
    cout << "TSC time: "         << MeasureStartStop(AccurateTSCTimer())*clock_cycle    << endl;
    cout << "Real time: "        << MeasureStartStop(HighResRealTimer())    << endl;
    cout << "Process CPU time: " << MeasureStartStop(HighResCPUTimer())     << endl;
    cout << "Thread CPU time: "  << MeasureStartStop(HighResThreadTimer())  << endl;
    cout << "All values in nanoseconds" << endl;
}

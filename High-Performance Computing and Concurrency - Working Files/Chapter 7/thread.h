#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>

#include <common.h>

// The Thread class can be subclassed to create an object whose
//  "Run" method is invoked in a separate thread.
//
//  Example:
//    class MyThread : public Threads::Thread {
//        public:
//        MyThread(const Threads::Thread::Options& options = Threads::Thread::Options()) : Threads::Thread(options) ...
//        protected:
//        virtual void Run() {
//            // Do the work here
//        }
//    };
//    ...
//    MyThread t(Threads::Thread::Options().set_stack_size(16384));
//    t.Start();
//    ...
//    t.Join();
//
// Note: threads started by the Thread class block all signals (except SIGSEGV
// and SIGBUS) by default. The implementer can choose to unblock certain
// signals using pthread_sigmask if necessary.

namespace Threads {
class Thread
{
    public:
    // Thread options.
    class Options {
        public:
        // Initialize options to default values.
        Options();

        // Mark the thread as joinable, which requires that the thread be joined in
        // order to clean up its state after it exits.
        // By default threads are joinable.
        Options& set_joinable(bool joinable) { joinable_ = joinable; return *this; }

        // Return whether the thread is joinable.
        bool joinable() const { return joinable_; }

        // Set the thread stack size (in bytes).
        // By default (or if stack_size == 0) the stack size equals the default
        // value (see set_default_stack_size() and default_stack_size()).
        Options& set_stack_size(size_t stack_size) { stack_size_ = stack_size; return *this; }

        // Return the thread stack size.
        size_t stack_size() const { return stack_size_; }

        private:
        size_t          stack_size_;            // Size of thread stack
        bool            joinable_;              // Thread is joinable?
    };

    // Create thread object.
    // The actual runtime thread is not created until Start() is called.  Start
    // cannot be called from this constructor because the started thread would
    // then race with initialization of any subclasses of Thread.
    // To simplify creating threads, the Thread::Options setter methods return
    // a reference to *this and can be chained:
    // 
    // MyThread() : Thread(Thread::Options().set_stack_size(1024*1024).set_joinable(true)) {...}
    //
    explicit Thread(const Thread::Options& options = Thread::Options());

    // Release all thread resources.
    virtual ~Thread();

    // Get the thread options.
    const Thread::Options& options() const { return options_; }

    // Get the thread's pthread id.
    pthread_t tid() const { return tid_; }

    // Create a runtime thread and invoke "this->Run()" as its payload.
    void Start();

    // Join with the running thread. Blocks until the thread terminates, unless
    // it has already terminated.  The results of multiple simultaneous calls
    // to Join are undefined.
    // The thread must have been started, and must have been marked as joinable
    // before being started.
    void Join();

    // Check if the thread was started. This does not check if the thread is
    // still running, only that Start() was called.
    bool started() const { return created_; }

    // Cancel the thread. This should be done with extreme caution: the thread
    // will be interrupted at an undefined moment, in asynchroneous fashion,
    // and then simply exit.  This is rarely the right solution. About the only
    // time when it makes sense to do so is if the program itself is exiting,
    // while some of the threads may be waiting on blocking I/O such as
    // sockets.
    // Joinable threads that were canceled must still be joined.
    void Cancel();

    protected:
    // Derived classes must implement this method and put the actual work there.
    virtual void Run() = 0;

    private:
    Thread::Options     options_;       // Thread configuration options
    pthread_t           tid_;           // Thread id
    bool                created_;       // Thread is created
    bool                needs_join_;    // Thread is joinable but not joined

    static void* Payload(void* arg);    // The actual work is done here

    DECLARE_NON_COPYABLE(Thread);
};
} // namespace Threads

#endif // THREAD_H_

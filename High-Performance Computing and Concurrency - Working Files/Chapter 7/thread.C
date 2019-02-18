#include <thread.h>

#include <alloca.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

using namespace std;

using namespace Threads;

// By default, threads block all signals. This flag disables signal blocking.
static bool disable_thread_signal_block = ::getenv("MT_DISABLE_THREAD_SIGNAL_BLOCK");

// Some platforms do not define PTHREAD_STACK_MIN
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif // PTHREAD_STACK_MIN 

// Return the smallest value divisible by alignment that is greater than the
// specified address.
static inline uintptr_t align_up(uintptr_t address, size_t alignment) {
    uintptr_t mask = alignment - 1;
    return (address + mask) & ~mask;
}

// Return the minimum valid stack size that can be passed to
// pthread_attr_setstacksize().
static size_t MinStackSize(size_t stack_size) {
  if (stack_size < PTHREAD_STACK_MIN) stack_size = PTHREAD_STACK_MIN;

  // Make stack size a a multiple of the system page size.
  static size_t page_size = sysconf(_SC_PAGESIZE);
  return align_up(stack_size, page_size);
}

// Thread options.
Thread::Options::Options()
    : stack_size_(0),
      joinable_(true)
{
}


Thread::Thread(const Thread::Options& options)
    : options_(options),
      tid_(0),
      created_(false),
      needs_join_(false)
{
}

Thread::~Thread() {
    if (needs_join_) {
        LOG_ERROR_NONFATAL << "Joinable thread was not joined - memory and other resources will be leaked!";
    }
}

// Create the thread and run the payload.
void Thread::Start() {
    CHECK(!created_) << "Start() called on a running thread!";
    created_ = true;
    needs_join_ = options_.joinable();

    // Prepare thread attributes.
    pthread_attr_t attr;
    CHECK_EQ(pthread_attr_init(&attr), 0);
    int detach = options_.joinable() ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED;
    CHECK_EQ(pthread_attr_setdetachstate(&attr, detach), 0);

    // Set up stack size. 
    // Use the system default size if stack size is not specified.
    size_t stack_size = 2048*1024;
    if (options_.stack_size() != 0) {
        stack_size = options_.stack_size();
    }

    stack_size = MinStackSize(stack_size);
    CHECK_EQ(pthread_attr_setstacksize(&attr, stack_size), 0)
        << ": specified stack size = " << stack_size
        << ", PTHREAD_STACK_MIN= " << PTHREAD_STACK_MIN;

    // Do the real work!
    int rc = pthread_create(&tid_, &attr, Payload, this);
    CHECK_EQ(rc, 0) << "Failed to start a thread: " << strerror(rc)
        << ((rc == ENOMEM) ? " Thread stack size may be too large." : "" );

    // Clean up attributes.
    CHECK_EQ(pthread_attr_destroy(&attr), 0);
}

// Join the thread (if it is joinable).
void Thread::Join() {
    CHECK(options_.joinable()) << "Attempting to join a detached thread!";
    CHECK(created_) << "Attempting to join a non-existing thread!";
    if (!needs_join_) return;
    int rc = pthread_join(tid_, NULL);
    CHECK_EQ(rc, 0) << "Failed to join a thread: " << strerror(rc)
        << ((rc == EDEADLK) ? " Deadlock, is the thread joining itself?" : "");
    needs_join_ = false;
}

// Cancel the thread.
void Thread::Cancel() {
    CHECK(created_) << "Attempting to cancel a non-existing thread!";
    int rc = pthread_cancel(tid_);
    CHECK_EQ(rc, 0) << "Failed to cancel a thread: " << strerror(rc);
}

// Thread payload, this is where actual work is done.
void* Thread::Payload(void* arg) {
    // Make sure that this thread does not get any signals.
    sigset_t set;
    if (!disable_thread_signal_block) { // Default, most signals are disabled
        sigfillset( &set );
        sigdelset( &set, SIGPROF );
        sigdelset( &set, SIGSEGV );
        sigdelset( &set, SIGBUS );
    } else { // Make sure that this thread does not get the SIGCLD signal from MGLS
        sigemptyset( &set );
        sigaddset( &set, SIGCHLD );
    }
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // Recover thread pointer.
    Thread* this_thread = static_cast<Thread*>(arg);

    // And now, the moment we have all been waiting for...
    this_thread->Run();

    // The Thread object may have been deleted by the Run() method, so it
    // should not be accessed beyond this point.
    this_thread = NULL;

    return NULL;
}


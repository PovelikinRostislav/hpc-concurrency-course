#include <unistd.h>
#include <pthread.h>
#include <iostream>

#include <thread.h>

using namespace std;

pthread_mutex_t M = PTHREAD_MUTEX_INITIALIZER;

typedef unsigned char T;
//typedef unsigned int T;

class MyThread : public Threads::Thread {
  public:
  MyThread(T* p, T d) : p_(p), d_(d) { Start(); }
  ~MyThread() { Join(); }
  private:
  virtual void Run() {
    for ( T i = 0; true; i += d_ ) {
      *p_ = i;
      if (*p_ != i) {
        pthread_mutex_lock(&M);
        cout << "Memory corruption!" << endl;
        pthread_mutex_unlock(&M);
        _exit(0);
      }
//pthread_mutex_lock(&M);
//cout << "p=" << (void*)(p_) << " i=" << size_t(i) << " *p=" << size_t(*p_) << endl;
//pthread_mutex_unlock(&M);
    }
  }
  volatile T* p_;
  T d_;
};

int main() {
  unsigned char* a = new unsigned char[1024];
  T* p1 = (T*)(a + 62);
  T* p2 = (T*)(a + 66);
  MyThread t1(p1, 1);
  MyThread t2(p2, -1);
}


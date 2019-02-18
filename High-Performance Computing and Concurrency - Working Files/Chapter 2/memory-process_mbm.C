#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <time.h>

#include <iostream>
using namespace std;

typedef long Word;

void __attribute__ ((noinline)) mark(Word* p, size_t N, size_t n) {
//cout << "n=" << n << " N=" << N << endl;
  for (size_t i = n; i < N; i += n) {
    p[n] = 0;
  }
}

template <size_t Nn>
void __attribute__ ((noinline)) mark1(Word* p, size_t N, const int (&nn)[Nn]) {
  for (size_t k = 0; k < Nn; ++k) {
    mark(p, N, nn[k]);
  }
}

template <size_t Nn>
void __attribute__ ((noinline)) mark2(Word* p, size_t N, const int (&nn)[Nn]) {
  const size_t DN = 16*1024/sizeof(Word);
  for (size_t N0 = 0; N0 < N; N0 += DN) {
    for (size_t k = 0; k < Nn; ++k) {
      mark(p + N0, DN, nn[k]);
    }
  }
}

int main() {
  const size_t N = 256UL*1024UL*1024UL;
  Word* p = new Word[N]; 
  static const int nn[] = { 2, 3, 5, 7, 11, 13, 17, 19 };
  for (size_t i = 0; i < N; ++i) p[i] = 1;
  static const size_t NR = 32;
  {
    time_t t0 = time(NULL);
    for (size_t i = 0; i < NR; ++i) {
      mark1(p, N, nn);
      cout << '.' << flush;
    }
    time_t t1 = time(NULL);
    cout << t1 - t0 << endl;
  }
  {
    time_t t0 = time(NULL);
    for (size_t i = 0; i < NR; ++i) {
      mark2(p, N, nn);
      cout << '.' << flush;
    }
    time_t t1 = time(NULL);
    cout << t1 - t0 << endl;
  }
  delete [] p;
}

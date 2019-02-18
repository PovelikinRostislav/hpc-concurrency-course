#include <atomic-forward-list.h>

#include <string.h>

#include "benchmark/benchmark.h"

#define ARGS(N) \
  ->Threads(N) \
  ->UseRealTime()

using namespace std;

#include <list_test_utils.h>

atomic_forward_list<entry_t> l;

#include <list_test.h>

# Compiler setup. Unit tests should, in general, be built with -g.
CXX = g++
# -fabi-version=6 is needed to avoid name conflicts between templates
# instantiated on SSE words of different length, such as __m128i and __m256i.
CXXFLAGS_COMMON = -Wall -Wextra -Werror -Wno-unused-local-typedefs -fabi-version=6
CXX0XFLAGS = -std=c++0x
CXXFLAGS_TARGET = -mavx
CXXFLAGS = -O4 $(INCLUDES) $(CXXFLAGS_COMMON) -pedantic $(CXXFLAGS_TARGET)
CXXFLAGS_BENCH = -O4 $(INCLUDES) -I$(GBENCH_INCLUDE_DIR) $(CXXFLAGS)

# Flags for unit tests: gtest.h does not compile with -pedantic.
CXXFLAGS_TEST = -g $(INCLUDES) -I$(GTEST_INCLUDE_DIR) $(CXXFLAGS_COMMON)

# All unit tests and test component libraries produced by this Makefile.
# Remember to add new tests you created to the list.
TESTS = 

TEST_LIBS = 

# All non-test binaries produced by this Makefile.
BINARIES = std_mutex_mbm std_mutex1_mbm std_mutex_queue_mbm std_mutex_queue1_mbm \
	   pthread_mutex_mbm \
	   spinlock_mbm spinlock1_mbm spinlock1_xchg_mbm spinlock1_atomic_mbm spinlock_ptr1_mbm \
	   spinlock_queue_mbm spinlock_queue1_mbm spinlock_ptr_queue_mbm spinlock_ptr_queue1_mbm \
	   atomic_mbm

# House-keeping build targets.

all : tests exec

tests : $(TESTS)

exec : $(BINARIES)

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                       GOOGLETEST SETUP - DO NOT CHANGE                      #
#
# Points to the root of Google Test, relative to where this file is.
GTEST_DIR = ../gtest
GTEST_LIB_DIR = $(GTEST_DIR)/lib
GTEST_INCLUDE_DIR = $(GTEST_DIR)/include
GTEST_LIBS = $(GTEST_LIB_DIR)/gtest.a
GTEST_LIBS_MAIN = $(GTEST_LIB_DIR)/gtest_main.a

#
#                    GOOGLE BENCHMARK SETUP - DO NOT CHANGE                   #
#
# Points to the root of Google Benchmark, relative to where this file is.
GBENCH_DIR = ../gbench
GBENCH_LIB_DIR = $(GBENCH_DIR)/lib
GBENCH_INCLUDE_DIR = $(GBENCH_DIR)/include
GBENCH_LIBS = $(GBENCH_LIB_DIR)/libbenchmark.a

#                              END OF GOOGLE SETUP                            #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                          BUILD SETUP - DO NOT CHANGE                        #
#
VPATH = .

INCLUDES = -I.

.PHONY : clean
clean :
	rm -f $(TESTS) $(BINARIES)

.PRECIOUS : $(TESTS)

#                               END OF BUILD SETUP                            #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                               MICRO-BENCHMARKS                              #
#

std_mutex_mbm : std_mutex_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

std_mutex1_mbm : std_mutex1_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

std_mutex_queue_mbm : std_mutex_queue_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

std_mutex_queue1_mbm : std_mutex_queue1_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

pthread_mutex_mbm : pthread_mutex_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock_mbm : spinlock_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock1_mbm : spinlock1_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock1_xchg_mbm : spinlock1_xchg_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock1_atomic_mbm : spinlock1_atomic_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock_ptr1_mbm : spinlock_ptr1_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock_queue_mbm : spinlock_queue_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock_queue1_mbm : spinlock_queue1_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock_ptr_queue_mbm : spinlock_ptr_queue_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

spinlock_ptr_queue1_mbm : spinlock_ptr_queue1_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

atomic_mbm : atomic_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

#                               END OF BINARIES                               #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                  UNIT TESTS                                 #

# All tests should link with either $(GTEST_LIBS) or $(GTEST_LIBS_MAIN),
# depending on whether it defines its own main() function.
# Use CXXFLAGS in all compilations except unit tests, use CXXFLAGS_TEST in unit tests.

#                               END OF UNIT TESTS                             #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

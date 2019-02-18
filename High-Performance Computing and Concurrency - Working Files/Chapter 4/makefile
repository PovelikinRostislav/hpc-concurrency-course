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
TESTS = thread_test 

TEST_LIBS = common.a timers.a tsc-timer.a thread.a atomic.a

# All non-test binaries produced by this Makefile.
BINARIES = thread_mbm threads_mbm increment1_mbm increment2_mbm

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

thread_mbm : thread_mbm.C thread.h thread.C common.h common.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

threads_mbm : threads_mbm.C thread.h thread.C common.h common.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

increment1_mbm : increment1_mbm.C thread.h thread.C common.h common.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

increment2_mbm : increment2_mbm.C thread.h thread.C common.h common.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@

#                               END OF BINARIES                               #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                  UNIT TESTS                                 #

# All tests should link with either $(GTEST_LIBS) or $(GTEST_LIBS_MAIN),
# depending on whether it defines its own main() function.
# Use CXXFLAGS in all compilations except unit tests, use CXXFLAGS_TEST in unit tests.

thread_test : thread_test.C thread.C thread.h common.C common.h
	$(CXX) $(^:%.h=) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -o $@ && ./$@

#                               END OF UNIT TESTS                             #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

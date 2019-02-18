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

# All non-test binaries produced by this Makefile.
BINARIES = \
memory-seq_mbm1 memory-seq_mbm2 memory-seq_mbm2t \
memory-write_mbm1 memory-write_mbm1a memory-write_mbm2 memory-write_mbm2t \
memory-write_mbm3 \
memory-increment_mbm1 memory-increment_mbm1a memory-increment_mbm2 memory-increment_mbm3 \
memory-process_mbm

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

memory-seq_mbm1 : memory-seq_mbm1.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-seq_mbm2 : memory-seq_mbm2.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-seq_mbm2t : memory-seq_mbm2t.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-write_mbm1 : memory-write_mbm1.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-write_mbm1a : memory-write_mbm1a.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-write_mbm2 : memory-write_mbm2.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-write_mbm2t : memory-write_mbm2t.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-write_mbm3 : memory-write_mbm3.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-increment_mbm1 : memory-increment_mbm1.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-increment_mbm1a : memory-increment_mbm1a.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-increment_mbm2 : memory-increment_mbm2.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-increment_mbm3 : memory-increment_mbm3.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

memory-process_mbm : memory-process_mbm.C
	$(CXX) $(^:%.h=) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

#                               END OF BINARIES                               #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

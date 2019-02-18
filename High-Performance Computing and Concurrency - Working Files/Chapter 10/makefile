# Compiler setup. Unit tests should, in general, be built with -g.
CXX = g++-5
CXX_TSAN = clang++-3.5
# -fabi-version=6 is needed to avoid name conflicts between templates
# instantiated on SSE words of different length, such as __m128i and __m256i.
CXXFLAGS_COMMON = -Wall -Wextra -Werror -Wno-unused-local-typedefs -fabi-version=6
CXX0XFLAGS = -std=c++0x
CXXFLAGS_TARGET = -mavx
CXXFLAGS = -O4 $(INCLUDES) $(CXXFLAGS_COMMON) -pedantic $(CXXFLAGS_TARGET)
CXXFLAGS_BENCH = -O4 $(INCLUDES) -I$(GBENCH_INCLUDE_DIR) $(CXXFLAGS)
CXXFLAGS_TSAN = -O2 -g $(INCLUDES) -Wno-deprecated-register -fsanitize=thread -fPIE -pie

# Flags for unit tests: gtest.h does not compile with -pedantic.
CXXFLAGS_TEST = -g $(INCLUDES) -I$(GTEST_INCLUDE_DIR) $(CXXFLAGS_COMMON)

# All unit tests and test component libraries produced by this Makefile.
# Remember to add new tests you created to the list.
TESTS = intr_shared_ptr_test intr_shared_ptr_mt_test \
        atomic_queue_test \
        concurrent_queue_test \
        atomic-forward-list_test

TEST_LIBS = 

# All non-test binaries produced by this Makefile.
BINARIES = test_mbm atomic_mbm cas_mbm casa_mbm spinlock_mbm spinlock_ptr_mbm mutex_mbm \
	intr_shared_ptr_tsan shared_ptr_mbm intr_shared_ptr_mbm atomic_shared_ptr_mbm shared_ptr_atomic_mbm \
        concurrent_queue_tsan concurrent_queue_mbm concurrent_std_queue_mbm \
        concurrent_queue_large_mbm concurrent_std_queue_large_mbm \
        lock_queue_mbm proto_atomic_queue1_mbm proto_atomic_queue2_mbm proto_atomic_queue3_mbm proto_atomic_queue3a_mbm proto_atomic_queue4_mbm proto_atomic_queue5_mbm proto_atomic_queue5a_mbm \
        lock_queue_large_mbm proto_atomic_queue1_large_mbm proto_atomic_queue5_large_mbm \
        atomic_queue1_mbm atomic_queue2_mbm \
	atomic_forward_list_mbm lock_forward_list_mbm mutex_forward_list_mbm

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

intr_shared_ptr_tsan : intr_shared_ptr_tsan.C intr_shared_ptr.h
	$(CXX_TSAN) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_TSAN) -lpthread -lrt -lm -o $@

test_mbm : test_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

atomic_mbm : atomic_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

cas_mbm : cas_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

casa_mbm : casa_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

spinlock_mbm : spinlock_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

spinlock_ptr_mbm : spinlock_ptr_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

mutex_mbm : mutex_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

shared_ptr_mbm : shared_ptr_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

intr_shared_ptr_mbm : intr_shared_ptr_mbm.C intr_shared_ptr.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

atomic_shared_ptr_mbm : atomic_shared_ptr_mbm.C atomic_shared_ptr.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) -Wno-deprecated-declarations $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -latomic -lrt -lm -o $@ 

shared_ptr_atomic_mbm : shared_ptr_atomic_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

atomic_queue1_mbm : atomic_queue1_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

atomic_queue2_mbm : atomic_queue2_mbm.C
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

lock_queue_mbm : lock_queue_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

lock_queue_large_mbm : lock_queue_large_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue1_mbm : proto_atomic_queue1_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue1_large_mbm : proto_atomic_queue1_large_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue2_mbm : proto_atomic_queue2_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue3_mbm : proto_atomic_queue3_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue3a_mbm : proto_atomic_queue3a_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue4_mbm : proto_atomic_queue4_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue5_mbm : proto_atomic_queue5_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue5_large_mbm : proto_atomic_queue5_large_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

proto_atomic_queue5a_mbm : proto_atomic_queue5a_mbm.C queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

concurrent_queue_mbm : concurrent_queue_mbm.C concurrent_queue.h queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

concurrent_std_queue_mbm : concurrent_std_queue_mbm.C concurrent_queue.h queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

concurrent_queue_large_mbm : concurrent_queue_large_mbm.C concurrent_queue.h queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

concurrent_std_queue_large_mbm : concurrent_std_queue_large_mbm.C concurrent_queue.h queue_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

concurrent_queue_tsan : concurrent_queue_tsan.C concurrent_queue.h atomic_queue1.h
	$(CXX_TSAN) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_TSAN) -lpthread -lrt -lm -o $@

atomic_forward_list_mbm : atomic_forward_list_mbm.C atomic-forward-list.h intr_shared_ptr.h list_test_utils.h list_test.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

lock_forward_list_mbm : lock_forward_list_mbm.C list_test_utils.h list_test.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

mutex_forward_list_mbm : mutex_forward_list_mbm.C list_test_utils.h list_test.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_BENCH) $(GBENCH_LIBS) -lpthread -lrt -lm -o $@ 

list_bm : list_bm.C atomic-forward-list.h intr_shared_ptr.h list_test_utils.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) -O4 $(INCLUDES) $(CXXFLAGS) -lpthread -lrt -lm -o $@ 

incr_bm : incr_bm.C 
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) -O4 $(INCLUDES) $(CXXFLAGS) -lpthread -lrt -lm -o $@ 

#                               END OF BINARIES                               #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                  UNIT TESTS                                 #

# All tests should link with either $(GTEST_LIBS) or $(GTEST_LIBS_MAIN),
# depending on whether it defines its own main() function.
# Use CXXFLAGS in all compilations except unit tests, use CXXFLAGS_TEST in unit tests.

intr_shared_ptr_test : intr_shared_ptr_test.C intr_shared_ptr.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -o $@ && ./$@

intr_shared_ptr_mt_test : intr_shared_ptr_mt_test.C intr_shared_ptr.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -o $@ && ./$@

atomic_queue_test : atomic_queue_test.C atomic_queue.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -o $@ && ./$@

concurrent_queue_test : concurrent_queue_test.C concurrent_queue.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -o $@ && ./$@

atomic-forward-list_test : atomic-forward-list_test.C atomic-forward-list.h intr_shared_ptr.h
	$(CXX) $(^:%.h=) $(CXX0XFLAGS) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -o $@ && ./$@

#                               END OF UNIT TESTS                             #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

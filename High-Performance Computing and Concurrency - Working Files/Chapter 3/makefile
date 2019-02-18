# Compiler setup. Unit tests should, in general, be built with -g.
CXX = g++
CXXFLAGS_COMMON = -Wall -Wextra -Werror -Wno-unused-local-typedefs
CXX0XFLAGS = -std=c++0x
CXXFLAGS_TARGET = -mavx
CXXFLAGS = -O4 $(INCLUDES) $(CXXFLAGS_COMMON) -pedantic $(CXXFLAGS_TARGET)

# Flags for unit tests: gtest.h does not compile with -pedantic.
CXXFLAGS_TEST = -g $(INCLUDES) -I$(GTEST_INCLUDE_DIR) $(CXXFLAGS_COMMON)

# All unit tests and test component libraries produced by this Makefile.
# Remember to add new tests you created to the list.
TESTS = timers_test tsc-timer_test

TEST_LIBS = 

# All non-test binaries produced by this Makefile.
BINARIES = timers_mbm

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

#                            END OF GOOGLETEST SETUP                          #
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

timers_mbm : timers_mbm.C timers.C timers.h tsc-timer.C tsc-timer.h
	$(CXX) $(CXXFLAGS) -lpthread -lrt $(^:%.h=) -o $@

#                               END OF BINARIES                               #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#                                  UNIT TESTS                                 #

# All tests should link with either $(GTEST_LIBS) or $(GTEST_LIBS_MAIN),
# depending on whether it defines its own main() function.
# Use CXXFLAGS in all compilations except unit tests, use CXXFLAGS_TEST in unit tests.

timers_test : timers_test.C timers.C timers.h 
	$(CXX) $(^:%.h=) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -lrt -lm -o $@ && ./$@

tsc-timer_test : tsc-timer_test.C tsc-timer.C tsc-timer.h timers.C timers.h
	$(CXX) $(^:%.h=) $(CXXFLAGS_TEST) $(GTEST_LIBS_MAIN) -lpthread -lrt -o $@ && ./$@


#                               END OF UNIT TESTS                             #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

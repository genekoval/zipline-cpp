project := zipline-cpp

STD := c++20

test.libs = gtest gmock pthread timber

files = $(include) $(src) Makefile VERSION

include mkbuild/base.mk

$($(test).objects): CXXFLAGS += -DTESTDIR='"$(build)"'

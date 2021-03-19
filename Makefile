project := zipline

STD := c++20

define test.libs
 gtest
 gtest_main
 gmock
 pthread
 timber
endef

include mkbuild/base.mk

$($(test).objects): CXXFLAGS += -DTESTDIR='"$(build)"'

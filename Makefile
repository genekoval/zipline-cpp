name := zipline

project := $(name)-cpp

STD := c++20

library := lib$(name)
$(library).type := shared
$(library).libs := fmt timber

install := $(library)
targets := $(install)

test.libs = $(name) crypto ext++ fmt gtest gmock netcore pthread ssl timber
test.deps = $(library)

install.directories = $(include)/$(name)

files = $(include) $(src) Makefile VERSION

include mkbuild/base.mk

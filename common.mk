#
# Copyright 2021 Santanu Sen. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License"). You may not use
# this file except in compliance with the License. You can obtain a copy
# in the file LICENSE in the source distribution.
#

CXXFORMAT = clang-format

CXXFLAGS := $(CXXFLAGS) \
    -I ../util \
	-Wall -Wextra -Werror \
	-Wmissing-include-dirs -Wshadow \
	-Wunreachable-code -Wredundant-decls \
	-Wcast-qual -Wcast-align -Wsign-promo \
	-Woverloaded-virtual -Wctor-dtor-privacy \
	-Wstrict-overflow=5 -Wswitch-default -Wundef \
	-fno-elide-constructors \
	-g

TOPTARGETS := all clean format

SRCS = $(wildcard *.cpp)
HDRS = $(wildcard *.hpp)

EXECS = $(SRCS:.cpp=.out )

all: $(EXECS)

clean:
	rm -f *.out *.ii

format:
	for srcfile in $(SRCS) $(HDRS) ; do $(CXXFORMAT) -i $${srcfile} ; done

.PHONY: $(TOPTARGETS)

.SUFFIXES: .ii .out

%.ii: %.cpp
	$(CXX) $(CXXFLAGS) -E -o $@ $<

%.out: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<


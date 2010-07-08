#TODO: Generalize it
#CXXFLAGS=-g
CXXFLAGS=-march=native -O3 -pipe -ansi -Wall -pedantic
PREFIX=$(HOME)

all: battery

battery: battery.cpp
	$(CXX) $(CXXFLAGS) -o battery battery.cpp

install: battery
	install battery $(PREFIX)/bin/

.PHONY: clean uninstall

uninstall: install
	rm $(PREFIX)/bin/battery

clean: battery
	rm battery

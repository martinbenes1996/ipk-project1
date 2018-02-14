
# Makefile
# Project manager file
# IPK project
# FIT VUT
# 2017/2018



# compile settings
cc = g++
defines = -DDEBUG_MODE
linkings = -lpthread -lm
flags = $(defines) -std=c++11 -O0 -g -pedantic -Wall -Wextra

all: ipk-client ipk-server

ipk-client: client.cpp
	$(cc) $(flags) $< -o $@

ipk-server: server.cpp
	$(cc) $(flags) $< -o $@


# clean
.PHONY: clean
clean:
	@echo "Cleaning generated files.";\
	rm -rf *~ *.o *.gch *.dep ipk-client #ipk-server

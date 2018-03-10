
# Makefile
# Project manager file
# IPK project
# FIT VUT
# 2017/2018



# compile settings
cc = g++
defines = -DDEBUG_MODE
linkings = -lpthread -lm
flags = $(defines) -std=c++14 -O2 -g -pedantic -Wall -Wextra

all: ipk-client ipk-server

ipk-client: client.cpp
	@echo "Building ipk-client.";\
	$(cc) $(flags) $< -o $@

ipk-server: server.cpp
	@echo "Building ipk-server.";\
	$(cc) $(flags) $< -o $@


# doc
.PHONY: doc
doc:
	@echo "Create documentation.";\
	cd doc && pdflatex dokumentace.tex && cd ..

# clean
.PHONY: clean
clean:
	@echo "Cleaning generated files.";\
	rm -rf *~ *.o *.gch *.dep ipk-client ipk-server doc/dokumentace.aux doc/dokumentace.log doc/dokumentace.pdf ipk.tar.gz

# zip
.PHONY: zip
zip:
	@echo "Zipping files.";\
	tar -zcvf ipk.tar.gz *.cpp *.h Makefile doc/dokumentace.tex doc/*.png> /dev/null 2> /dev/null

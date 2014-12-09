SIMPLEJSON_INCLUDE=lib/SimpleJSON/src

JSON_EXT_OBJS=lib/SimpleJSON/obj/JSON.o lib/SimpleJSON/obj/JSONValue.o

default: all

all: matching 

clean:
	rm -f obj/*.o bin/*
	cd lib/SimpleJSON; make clean

obj/matching.o: src/matching.cpp
	@test -d $(@D) || mkdir -p $(@D)
	g++ -O2 -I. -I$(SIMPLEJSON_INCLUDE) -c $< -o $@

matching: obj/matching.o json
	@test -d bin || mkdir -p bin
	g++ -O2 -o bin/matching obj/matching.o $(JSON_EXT_OBJS)

json:
	cd lib/SimpleJSON; make


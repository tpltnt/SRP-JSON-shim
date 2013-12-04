CC=clang
.PHONY: clean


srp_datatypes.o: SRP/srp_datatypes.c
	$(CC) -c SRP/srp_datatypes.c -o srp_datatypes.o
data-parser.o: data-parser.c 
	$(CC) -c data-parser.c -o data-parser.o

all: srp_datatypes.o data-parser.o
	$(CC) srp_datatypes.o data-parser.o `pkg-config --cflags --libs jansson` -o simulation-proxy

testdata: testdata.json testdata2.json testdata3.json
	cat testdata.json | python3 -m json.tool
	cat testdata2.json | python3 -m json.tool
	cat testdata3.json | python3 -m json.tool

test: all testdata
	./simulation-proxy testdata.json
	./simulation-proxy testdata2.json
	./simulation-proxy testdata3.json

clean:
	rm srp_datatypes.o
	rm data-parser.o

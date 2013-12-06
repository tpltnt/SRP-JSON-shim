CC=clang
.PHONY: clean


srp.o: SRP/srp.c
	$(CC) -c SRP/srp.c -o srp.o

srp_datatypes.o: SRP/srp_datatypes.c
	$(CC) -c SRP/srp_datatypes.c -o srp_datatypes.o
data-parser.o: data-parser.c 
	$(CC) -c data-parser.c -o data-parser.o
main.o: main.c
	$(CC) -c main.c -o main.o

all: srp_datatypes.o data-parser.o main.o
	$(CC) srp.o srp_datatypes.o data-parser.o main.o `pkg-config --cflags --libs jansson` -o simulation-proxy

testdata: testdata.json testdata2.json testdata3.json
	cat testdata.json | python3 -m json.tool
	cat testdata2.json | python3 -m json.tool
	cat testdata3.json | python3 -m json.tool

test: all testdata
	./simulation-proxy testdata.json
	./simulation-proxy testdata2.json
	./simulation-proxy testdata3.json

clean:
	rm srp.o
	rm srp_datatypes.o
	rm data-parser.o
	rm main.o

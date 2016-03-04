CXXFLAGS=-O2 -Wall -g

all: vcdiff

vcdiff: main.o comparator.o link.o scope.o tokenizer.o variable.o vcdfile.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm *.o

.PHONY: clean

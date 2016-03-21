CXXFLAGS=-O2 -Wall -g
EXE=vcdiff

all: $(EXE)

$(EXE): main.o comparator.o link.o scope.o tokenizer.o variable.o vcdfile.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm *.o

install:
	cp $(EXE) /usr/local/bin

uninstall:
	rm /usr/local/bin/$(EXE)

.PHONY: clean install uninstall

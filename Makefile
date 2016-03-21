CXXFLAGS=-O2 -Wall
BIN=vcdiff

SRCS=main.cc comparator.cc link.cc scope.cc tokenizer.cc variable.cc vcdfile.cc
OBJS=$(SRCS:.cc=.o)
DEPS=$(OBJS:.o=.d)

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -MMD $<

clean:
	rm $(BIN) $(OBJS) $(DEPS) || true

install:
	cp $(BIN) /usr/local/bin

uninstall:
	rm /usr/local/bin/$(BIN)

-include $(DEPS)

.PHONY: clean install uninstall

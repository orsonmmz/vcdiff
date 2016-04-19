CXXFLAGS = -O2 -Wall
BIN = vcdiff

SRCS = main.cc comparator.cc link.cc scope.cc tokenizer.cc variable.cc vcdfile.cc
OBJS = $(SRCS:.cc=.o)
DEPS = $(OBJS:.o=.d)
DESTDIR ?= /usr

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -MMD $<

clean:
	rm $(BIN) $(OBJS) $(DEPS) || true

install:
	mkdir -p $(DESTDIR)/bin
	cp $(BIN) $(DESTDIR)/bin

uninstall:
	rm $(DESTDIR)/bin/$(BIN)

-include $(DEPS)

.PHONY: clean install uninstall

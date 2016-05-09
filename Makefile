CXXFLAGS = -O2 -Wall
BIN = vcdiff

SRCS = main.cc comparator.cc link.cc scope.cc tokenizer.cc value.cc variable.cc vcdfile.cc
OBJS = $(SRCS:.cc=.o)
DEPS = $(OBJS:.o=.d)

PREFIX ?= /usr
BINDIR = $(DESTDIR)$(PREFIX)/bin

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -MMD $<

clean:
	rm $(BIN) $(OBJS) $(DEPS) || true

install:
	mkdir -p $(BINDIR)
	cp $(BIN) $(BINDIR)

uninstall:
	rm $(BINDIR)/$(BIN)

-include $(DEPS)

.PHONY: clean install uninstall

MAIN := tyrant_optimize
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,obj/%.o,$(SRCS))

CPPFLAGS := -Wall -std=gnu++11 -O3
LDFLAGS := -lboost_system -lboost_thread -lboost_filesystem

all: $(MAIN)

obj/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(MAIN): $(OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(OBJS)

clean:
	rm -f $(MAIN) obj/*.o

MAIN := tu_optimize
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,obj/%.o,$(SRCS))
INCS := $(wildcard *.h)

CPPFLAGS := -Wall -Werror -std=gnu++11 -O3
LDFLAGS := -lboost_system -lboost_thread -lboost_filesystem

all: $(MAIN)

obj/%.o: %.cpp ${INCS}
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(MAIN): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

clean:
	del /q $(MAIN).exe obj\*.o

release:
	xcopy /y tu_optimize.exe tu_optimize
	xcopy /y rapidxml_license.txt tu_optimize
	xcopy /y tu_optimize_license.txt tu_optimize
	xcopy /y README.md tu_optimize\readme.txt
	xcopy /y data tu_optimize\data

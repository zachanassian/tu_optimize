MAIN := tu_optimize
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,obj/%.o,$(SRCS))
INCS := $(wildcard *.h)

CPPFLAGS := -Wall -Werror -std=gnu++11 -O3
LDFLAGS := -lboost_system -lboost_thread -lboost_filesystem

all: $(MAIN)

obj/%.o: %.cpp ${INCS}
	$(CXX) $(CPPFLAGS) -o $@ -c $<

obj/icon.res: icon.rc
	windres icon.rc -O coff -o obj/icon.res

$(MAIN): $(OBJS) obj/icon.res
	$(CXX) -o $@ $(OBJS) obj/icon.res $(LDFLAGS)

clean:
	del /q $(MAIN).exe obj\*.* tu_optimize\*.* tu_optimize\data\*.*

release:
	xcopy /y tu_optimize.exe tu_optimize
	xcopy /y rapidxml_license.txt tu_optimize
	xcopy /y tu_optimize_license.txt tu_optimize
	xcopy /y README.md tu_optimize\readme.txt
	xcopy /y data\cardabbrs_template.txt tu_optimize\data
	xcopy /y data\customdecks_template.txt tu_optimize\data
	xcopy /y data\ownedcards_template.txt tu_optimize\data
	xcopy /y data\cards.xml tu_optimize\data
	xcopy /y data\missions.xml tu_optimize\data
	"%ProgramFiles%\AutoHotkey\Compiler\Ahk2Exe.exe" /in SimpleTUOptimizeStarter.ahk /out tu_optimize/SimpleTUOptimizeStarter.exe /icon tu_optimize.ico

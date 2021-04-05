CXX = u++
CXXFLAGS = -multi -Wall -Wextra -MMD

COMPONENTS = bank cardoffice conductor config driver global groupoff nameserver \
             parent printer student timer train trainstop watcard
OBJECTS = ${COMPONENTS:%=target/%.o}
EXEC = lrt

DEPENDS = ${OBJECTS:.o=.d}

.PHONY : all

all : ${EXEC}

${EXEC} : ${OBJECTS}
	${CXX} ${CXXFLAGS} $^ -o $@

-include ${DEPENDS}

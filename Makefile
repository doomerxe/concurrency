CXX = u++
CXXFLAGS = -multi -Wall -Wextra -MMD

COMPONENTS = bank cardoffice conductor config driver global groupoff nameserver \
             parent printer student timer train trainstop watcard
OBJECTS = ${COMPONENTS:%=target/%.o}
EXEC = lrt

DEPENDS = ${OBJECTS:.o=.d}

CURRENT = bank driver

CURRENTOBJECTS = ${CURRENT:%=target/%.o}

CURRENTDEPENDS = ${CURRENTOBJECTS:.o=.d}

.PHONY : all

all : ${EXEC}

${EXEC} : ${OBJECTS}
	${CXX} ${CXXFLAGS} $^ -o $@

target/%.o : src/%.cc
	${CXX} ${CXXFLAGS} -c $< -o $@

-include ${DEPENDS}

#############################################################

clean :						# remove files that can be regenerated
	rm -f ${CURRENTDEPENDS} ${CURRENTOBJECTS} ${EXEC}
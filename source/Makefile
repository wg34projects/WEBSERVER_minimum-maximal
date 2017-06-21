# makefile el16b005 www.dwell.at webserver tool
# clean included from Mr. Horauers makefile

# sources

SOURCE = server.h server.c functions.c incoming.c outgoing.c

# target

TARGET = webserver

# flags

CXX      = gcc
CXXFLAGS = -Wall --pedantic -Wcpp -lm -g

# create

$(TARGET): $(SOURCE)
	$(CXX) $(SOURCE) $(CXXFLAGS) -o $(TARGET) 

.PHONY: clean
clean:
	@rm -rf $(OBJECTS) $(TARGET) core *~


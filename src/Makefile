CXX		     = gcc
BIN_SERVER = argusd
BIN_CLIENT = argus
CXXFLAGS	 = -Wall -Wextra -g
####################################################################################
BIN_DIR		 = ../bin
SRC_SERVER = argusd.c
SRC_CLIENT = argus.c
SRC_LIB		 = lib.c
####################################################################################

all: argusd argus

argusd: $(SRC_SERVER) $(SRC_LIB)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(BIN_SERVER) $(SRC_SERVER) $(SRC_LIB)

argus: $(SRC_CLIENT) $(SRC_LIB)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(BIN_CLIENT) $(SRC_CLIENT) $(SRC_LIB)

clean:
	rm -f $(BIN_DIR)/*

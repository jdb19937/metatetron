OBJ = main.o view.o blob.o

HDR = useful.hh view.hh world.hh blob.hh

CXXFLAGS = -O6
LDFLAGS = -lSDL -lm -lpthread

main: $(OBJ)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LDFLAGS)

$(OBJ): $(HDR)

.PHONY: clean
clean:
	rm -f $(OBJ) main

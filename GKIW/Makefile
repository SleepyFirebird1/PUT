CXX=clang++
CXXFLAGS=-Iinclude -I/opt/homebrew/include
LDFLAGS=-L/opt/homebrew/lib
LIBS=-lglfw -lGLEW -framework OpenGL
HEADERS=$(wildcard include/*.h)
FILES=$(wildcard src/*.cpp)
TARGET=bin/main_file

$(TARGET): $(FILES) $(HEADERS) | bin
	$(CXX) -o $(TARGET) $(FILES) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

bin:
	mkdir -p bin

main_file: $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

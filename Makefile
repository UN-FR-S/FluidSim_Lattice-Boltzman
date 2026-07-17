CXX = g++
CXXFLAGS = -std=c++17 -O3
TEST_LIBS = -lgtest
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Quell- und Ziel-Dateien
MAIN_SRC = ArrayLatticeMain.cpp
TEST_SRC = ArrayLatticeTest.cpp
MAIN_BIN = ArrayLatticeMain
TEST_BIN = ArrayLatticeTest

# Default-Target
all: $(MAIN_BIN)

# Hauptprogramm
$(MAIN_BIN): $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Testprogramm
$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(TEST_LIBS)

# Nur Tests ausführen
test: $(TEST_BIN)
	./$(TEST_BIN)

# Nur Hauptprogramm ausführen (nach Bedarf neu bauen)
run: $(MAIN_BIN)
	./$(MAIN_BIN)

# Checkstyle mit clang-format (nur prüfen)
checkstyle:
	clang-format-14 *.cpp --dry-run -Werror

# Formatieren der Dateien
format:
	clang-format -i *.cpp

# Aufräumen
clean:
	rm -f $(MAIN_BIN) $(TEST_BIN)

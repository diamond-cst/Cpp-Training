# Makefile for 21 Card Trick Project
# 21张牌魔术项目的Makefile

CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Wpedantic -I./include
LDFLAGS =

# 目录 (Directories)
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
SAVE_DIR = saves

# 源文件 (Source files)
SOURCES = $(SRC_DIR)/Card.cpp \
          $(SRC_DIR)/TwentyOneCardTrick.cpp \
          $(SRC_DIR)/TwentySevenCardTrick.cpp \
          $(SRC_DIR)/ConfigurableCardTrick.cpp \
          $(SRC_DIR)/Utils.cpp \
          $(SRC_DIR)/PileChoice.cpp \
          $(SRC_DIR)/ReplayManager.cpp \
          $(SRC_DIR)/NetworkGame.cpp \
          $(SRC_DIR)/Leaderboard.cpp

# 目标文件 (Object files)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# 可执行文件 (Executables)
CONSOLE_TARGET = magic_trick_console
ENHANCED_TARGET = magic_trick_enhanced
TEST_TARGET = test_program
GUI_TARGET = magic_trick_gui

# 默认目标 (Default target)
.PHONY: all
all: enhanced

# 控制台版本 (Console version)
.PHONY: console
console: $(CONSOLE_TARGET)

$(CONSOLE_TARGET): $(OBJECTS) $(BUILD_DIR)/main_console.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "控制台版本编译完成！(Console version compiled!)"

# 增强版本 (Enhanced version)
.PHONY: enhanced
enhanced: $(ENHANCED_TARGET)

$(ENHANCED_TARGET): $(OBJECTS) $(BUILD_DIR)/main_enhanced.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "增强版本编译完成！(Enhanced version compiled!)"

# 测试程序 (Test program)
.PHONY: test
test: $(TEST_TARGET)

$(TEST_TARGET): $(BUILD_DIR)/Card.o $(BUILD_DIR)/TwentyOneCardTrick.o $(BUILD_DIR)/Utils.o $(BUILD_DIR)/PileChoice.o $(BUILD_DIR)/ReplayManager.o $(BUILD_DIR)/test_main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "测试程序编译完成！(Test program compiled!)"

# GUI版本 (GUI version)
.PHONY: gui
gui:
	@if command -v qmake6 >/dev/null 2>&1; then \
		mkdir -p $(BUILD_DIR)/qt_gui && cd $(BUILD_DIR)/qt_gui && qmake6 ../../gui/qt_magic_trick.pro && $(MAKE); \
	elif command -v qmake >/dev/null 2>&1; then \
		mkdir -p $(BUILD_DIR)/qt_gui && cd $(BUILD_DIR)/qt_gui && qmake ../../gui/qt_magic_trick.pro && $(MAKE); \
	else \
		echo "未找到Qt qmake/qmake6。macOS可执行: brew install qt"; \
		echo '安装后如qmake不在PATH，可执行: export PATH="/opt/homebrew/opt/qt/bin:$$PATH"'; \
		echo "然后重新运行: make gui"; \
		exit 1; \
	fi

# 编译目标文件 (Compile object files)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 创建构建目录 (Create build directory)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(SAVE_DIR)

# 清理 (Clean)
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*.o
	rm -f $(CONSOLE_TARGET) $(ENHANCED_TARGET) $(TEST_TARGET) $(GUI_TARGET)
	@echo "清理完成！(Cleaned!)"

# 运行增强版本 (Run enhanced version)
.PHONY: run
run: enhanced
	./$(ENHANCED_TARGET)

# 运行控制台版本 (Run console version)
.PHONY: run-console
run-console: console
	./$(CONSOLE_TARGET)

# 运行测试 (Run tests)
.PHONY: run-test
run-test: test
	./$(TEST_TARGET)

# 调试编译 (Debug build)
.PHONY: debug
debug: CXXFLAGS += -g -O0
debug: clean console
	@echo "调试版本编译完成！(Debug version compiled!)"

# 发布编译 (Release build)
.PHONY: release
release: CXXFLAGS += -O3 -DNDEBUG
release: clean console
	@echo "发布版本编译完成！(Release version compiled!)"

# 帮助 (Help)
.PHONY: help
help:
	@echo "可用目标 (Available targets):"
	@echo "  make all         - 编译所有 (默认：增强版本)"
	@echo "  make console     - 编译控制台版本"
	@echo "  make enhanced    - 编译增强版本（推荐）"
	@echo "  make test        - 编译测试程序"
	@echo "  make gui         - 编译Qt GUI版本"
	@echo "  make run         - 编译并运行增强版本"
	@echo "  make run-console - 编译并运行控制台版本"
	@echo "  make run-test    - 编译并运行测试程序"
	@echo "  make debug       - 编译调试版本"
	@echo "  make release     - 编译发布版本"
	@echo "  make clean       - 清理编译文件"
	@echo "  make help        - 显示此帮助信息"

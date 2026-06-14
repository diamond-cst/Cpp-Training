# 21张牌魔术 (21 Card Trick)

一个用C++实现的经典21张牌魔术模拟程序，展示面向对象编程、模板、运算符重载和异常处理等高级C++特性。

A classic 21 card magic trick simulation implemented in C++, demonstrating advanced C++ features including OOP, templates, operator overloading, and exception handling.

## 项目简介 (Project Overview)

这是一个C++课程实训项目，实现了经典的"21张牌魔术"。魔术原理：
- 从21张牌中让观众记住一张
- 将牌分成3堆（每堆7张）
- 询问观众记住的牌在哪一堆
- 将选中的牌堆放在中间，重新合并
- 重复3次后，观众记住的牌必定在第11张（索引10）

This is a C++ training project implementing the classic "21 Card Trick". The magic principle:
- Have the audience remember one card from 21 cards
- Deal cards into 3 piles (7 cards each)
- Ask which pile contains the remembered card
- Place the chosen pile in the middle and merge
- After 3 rounds, the remembered card will always be at position 11 (index 10)

## 功能特性 (Features)

### 已实现 (Implemented) ✅
- ✅ **核心魔术算法** - 完整的21张牌和27张牌魔术流程
- ✅ **面向对象设计** - 使用继承和多态
- ✅ **模板类** - Deck<T>模板实现
- ✅ **运算符重载** - 算术、比较、流运算符
- ✅ **异常处理** - 自定义异常层次结构
- ✅ **内存管理** - 深拷贝、移动语义
- ✅ **保存/加载** - 游戏状态持久化（5个存档槽位）
- ✅ **标准扑克牌** - 支持花色和点数显示
- ✅ **彩色控制台** - ANSI颜色支持
- ✅ **排行榜系统** - 前10名玩家记录
- ✅ **计分系统** - +10正确，-5错误
- ✅ **动画效果** - 逐字打印、等待动画
- ✅ **多种魔术** - 21张和27张牌变体
- ✅ **智能指针** - 使用unique_ptr管理对象

### 开发中 (In Development)
- 🚧 **计分系统** - 排行榜和得分追踪
- 🚧 **彩色显示** - ANSI颜色支持
- 🚧 **动画效果** - 发牌动画
- 🚧 **多种魔术** - 27张牌等变体
- 🚧 **GUI界面** - Qt图形界面
- 🚧 **音效** - 发牌和揭示音效

## 编译和运行 (Build and Run)

### 前置要求 (Prerequisites)
- C++14或更高版本编译器 (g++, clang++, MSVC)
- Make (可选，用于简化编译)
- CMake 3.10+ (可选)

### 使用Makefile编译 (Build with Makefile)

```bash
# 编译增强版本（推荐）
make enhanced

# 编译控制台版本
make console

# 编译测试程序
make test

# 编译并运行增强版本
make run

# 编译并运行测试
make run-test

# 编译调试版本
make debug

# 编译发布版本
make release

# 清理编译文件
make clean

# 查看帮助
make help
```

### 使用g++直接编译 (Build with g++ directly)

```bash
g++ -std=c++14 -I./include -o magic_trick_console \
    src/Card.cpp \
    src/TwentyOneCardTrick.cpp \
    src/main_console.cpp \
    -Wall -Wextra
```

### 使用CMake编译 (Build with CMake)

```bash
mkdir build
cd build
cmake .. -DBUILD_CONSOLE=ON
make
```

### 运行程序 (Run the program)

```bash
# 运行增强版本（推荐）
./magic_trick_enhanced

# 运行控制台版本
./magic_trick_console

# 运行测试程序
./test_program
```

## 项目结构 (Project Structure)

```
Cpp_traning/
├── include/              # 头文件 (Header files)
│   ├── Card.h           # 卡牌类
│   ├── Deck.h           # 牌堆模板类
│   ├── MagicTrick.h     # 魔术抽象基类
│   ├── TwentyOneCardTrick.h  # 21张牌魔术
│   └── Exceptions.h     # 自定义异常
├── src/                 # 源文件 (Source files)
│   ├── Card.cpp
│   ├── TwentyOneCardTrick.cpp
│   └── main_console.cpp # 控制台主程序
├── assets/              # 资源文件 (Assets)
│   ├── sounds/          # 音效
│   └── images/          # 图片
├── saves/               # 存档目录 (Save files)
├── docs/                # 文档 (Documentation)
├── build/               # 构建目录 (Build directory)
├── CMakeLists.txt       # CMake配置
├── Makefile             # Make配置
└── README.md            # 本文件
```

## 类设计 (Class Design)

### 核心类 (Core Classes)

1. **Card** - 卡牌类
   - 支持标准扑克牌（花色+点数）
   - 支持数字牌（1-21）
   - 运算符重载：==, !=, <, >, <<, >>

2. **Deck<T>** - 牌堆模板类
   - 动态数组管理
   - 深拷贝和移动语义
   - 运算符重载：+, -, *, ==, !=, []

3. **MagicTrick** - 魔术抽象基类
   - 定义魔术接口
   - 支持多态

4. **TwentyOneCardTrick** - 21张牌魔术实现
   - 完整的魔术算法
   - 保存/加载功能
   - 计分系统

### 异常层次 (Exception Hierarchy)

```
MagicTrickException (基类)
├── InvalidCardException
├── OutOfBoundsException
├── FileIOException
├── InvalidGameStateException
└── InvalidInputException
```

## 使用说明 (Usage)

### 开始新游戏 (Start New Game)

1. 运行程序并选择"开始新游戏"
2. 输入你的名字
3. 选择是否使用彩色显示
4. 在心中记住一张牌
5. 每轮告诉程序你的牌在哪一堆（1, 2或3）
6. 三轮后程序会揭示你记住的牌

### 保存和加载 (Save and Load)

- 游戏结束后可以选择保存到5个槽位之一
- 从主菜单选择"加载游戏"恢复进度
- 存档文件保存在`saves/`目录

## 开发计划 (Development Roadmap)

### Phase 1: 核心功能 ✅ (已完成)
- [x] Card类实现
- [x] Deck模板类
- [x] 21张牌魔术算法
- [x] 基本控制台界面
- [x] 保存/加载功能

### Phase 2: 增强功能 (进行中)
- [ ] 计分系统和排行榜
- [ ] ANSI颜色支持
- [ ] 发牌动画效果
- [ ] 27张牌魔术变体
- [ ] 迫牌魔术变体

### Phase 3: 高级功能
- [ ] Qt图形界面
- [ ] 卡牌图片显示
- [ ] 音效支持
- [ ] 鼠标点击交互
- [ ] 动画效果

### Phase 4: 额外功能
- [ ] 网络对战
- [ ] 回放系统
- [ ] PDF导出

## 技术亮点 (Technical Highlights)

1. **模板编程** - Deck<T>支持任意类型
2. **运算符重载** - 直观的牌堆操作
3. **RAII** - 自动内存管理
4. **移动语义** - 高效的资源转移
5. **异常安全** - 完善的错误处理
6. **多态** - 基于接口的设计

## 测试 (Testing)

```bash
# 运行基本测试
./magic_trick_console

# 测试场景：
# 1. 完成完整的魔术流程
# 2. 保存游戏到槽位1
# 3. 退出并重新加载
# 4. 测试无效输入处理
```

## 贡献 (Contributing)

这是一个教育项目，欢迎提出改进建议！

This is an educational project. Suggestions for improvements are welcome!

## 许可证 (License)

本项目仅用于教育目的。

This project is for educational purposes only.

## 作者 (Author)

软件工程专业学生 - C++课程实训项目

Software Engineering Student - C++ Course Training Project

## 致谢 (Acknowledgments)

感谢经典的21张牌魔术为本项目提供灵感！

Thanks to the classic 21 card trick for inspiring this project!

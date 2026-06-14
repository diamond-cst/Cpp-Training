# 快速开始指南 (Quick Start Guide)

## 21张牌魔术项目

### 5分钟快速上手

#### 1. 编译项目

```bash
# 进入项目目录
cd /Users/diamond/Desktop/Cpp_traning

# 编译控制台版本
make console

# 或使用g++直接编译
g++ -std=c++14 -I./include -o magic_trick_console \
    src/Card.cpp src/TwentyOneCardTrick.cpp src/main_console.cpp
```

#### 2. 运行测试

```bash
# 编译测试程序
g++ -std=c++14 -I./include -o test_program \
    src/Card.cpp src/TwentyOneCardTrick.cpp src/test_main.cpp

# 运行测试
./test_program
```

#### 3. 运行主程序

```bash
./magic_trick_console
```

### 项目结构一览

```
Cpp_traning/
├── include/          # 头文件（5个核心类）
├── src/              # 源文件（3个实现 + 2个主程序）
├── saves/            # 游戏存档
├── docs/             # 项目文档
├── Makefile          # 编译配置
└── README.md         # 项目说明
```

### 核心类说明

1. **Card** - 卡牌类
   - 支持标准扑克牌（♠♥♣♦）
   - 完整的运算符重载

2. **Deck<T>** - 牌堆模板类
   - 动态内存管理
   - 支持合并、复制等操作

3. **TwentyOneCardTrick** - 21张牌魔术
   - 完整的魔术算法
   - 保存/加载功能

### 常用命令

```bash
# 编译
make console

# 运行
make run

# 清理
make clean

# 查看帮助
make help
```

### 下一步

1. 查看 `README.md` 了解详细信息
2. 查看 `docs/SUMMARY.md` 了解项目状态
3. 查看 `docs/PROGRESS.md` 了解开发计划

### 需要帮助？

- 查看代码注释（中英文双语）
- 运行测试程序验证功能
- 阅读项目文档

---

**项目状态**: Phase 1 完成 ✅
**测试状态**: 所有测试通过 ✅
**编译状态**: 无错误无警告 ✅

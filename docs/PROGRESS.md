# 21张牌魔术项目 - 开发进度报告

## 项目信息
- **项目名称**: 21张牌魔术 (21 Card Trick)
- **开发语言**: C++14
- **目标分数**: 100分 (Task 4)
- **当前状态**: Phase 1 完成，Phase 2-4 进行中

## 已完成功能 (Completed Features)

### ✅ Phase 1: 核心类实现 (Task 1 - 65分基础)

1. **异常处理系统** (`include/Exceptions.h`)
   - MagicTrickException 基类
   - InvalidCardException - 无效卡牌
   - OutOfBoundsException - 越界访问
   - FileIOException - 文件IO错误
   - InvalidGameStateException - 无效游戏状态
   - InvalidInputException - 无效输入

2. **Card类** (`include/Card.h`, `src/Card.cpp`)
   - 支持标准扑克牌（花色：♠♥♣♦，点数：A-K）
   - 支持数字牌模式（1-21）
   - 运算符重载：
     - 比较运算符：==, !=, <, >, <=, >=
     - 流运算符：<<, >>
   - 颜色支持（ANSI颜色代码）
   - toString()方法用于格式化输出

3. **Deck模板类** (`include/Deck.h`)
   - 模板实现：`Deck<T>`
   - 动态数组管理（手动内存管理）
   - 深拷贝构造函数
   - 移动构造函数和移动赋值（C++11）
   - 运算符重载：
     - 算术运算符：+ (合并), - (移除), * (复制)
     - 比较运算符：==, !=
     - 下标运算符：[]
     - 流运算符：<<, >>
   - 核心功能：addCard, removeCard, shuffle, clear

4. **MagicTrick抽象基类** (`include/MagicTrick.h`)
   - 纯虚函数接口
   - 支持多态设计
   - 定义魔术标准接口

5. **TwentyOneCardTrick类** (`include/TwentyOneCardTrick.h`, `src/TwentyOneCardTrick.cpp`)
   - 完整的21张牌魔术算法实现
   - 初始化21张标准扑克牌
   - 发牌到3堆（每堆7张）
   - 重新整理算法（选中的堆放中间）
   - 3轮循环后揭示第11张牌
   - 保存/加载游戏状态（二进制格式）
   - 计分系统（+10正确，-5错误）

6. **控制台主程序** (`src/main_console.cpp`)
   - 友好的中英文双语界面
   - 主菜单系统
   - 新游戏流程
   - 保存/加载功能（5个存档槽位）
   - 异常处理

7. **构建系统**
   - CMakeLists.txt - CMake配置
   - Makefile - Make配置
   - 支持多种编译方式

8. **文档**
   - README.md - 完整的项目说明
   - .gitignore - Git忽略配置
   - 中英文双语注释

## 待实现功能 (TODO Features)

### 🚧 Phase 2: 增强功能 (Task 2 - 80分)

1. **SaveManager类** - 统一的存档管理
   - 多玩家存档支持
   - 存档列表显示
   - 存档验证

2. **暂停/恢复功能**
   - P键暂停
   - R键恢复
   - 自动保存

3. **魔术变体**
   - TwentySevenCardTrick (27张牌)
   - ForcedCardTrick (迫牌魔术)
   - 通过基类指针调用（多态演示）

4. **增强运算符重载**
   - 完善所有运算符的实际应用场景

### 🚧 Phase 3: 高级功能 (Task 3 - 90分)

1. **Leaderboard类** - 排行榜系统
   - 保存前10名玩家
   - 分数、名字、日期
   - 文件持久化

2. **Utils工具类**
   - ANSI颜色函数
   - 动画效果（发牌延迟）
   - 清屏函数
   - 格式化输出

3. **Config配置类**
   - 可配置牌堆大小（15, 21, 27）
   - 显示模式设置
   - 颜色开关

4. **游戏模式**
   - 观众模式（正确+10，错误-5，连胜追踪）
   - 魔术师练习模式（计时评分）

5. **控制台增强**
   - 完整的ANSI颜色支持
   - 发牌动画
   - 边框和格式化输出

### 🚧 Phase 4: GUI和额外功能 (Task 4 - 100分)

1. **Qt GUI实现**
   - MainWindow - 主窗口
   - CardWidget - 卡牌显示组件
   - GameWidget - 游戏区域
   - LeaderboardDialog - 排行榜窗口
   - SaveLoadDialog - 存档对话框

2. **资源文件**
   - 52张卡牌图片
   - 卡牌背面图片
   - 音效文件（deal.wav, flip.wav, reveal.wav等）

3. **交互功能**
   - 鼠标点击选择牌堆
   - 动画效果（QPropertyAnimation）
   - 音效播放（QSound/QMediaPlayer）

4. **额外创新**
   - 回放系统
   - 统计数据
   - 成就系统

## 技术亮点 (Technical Highlights)

### 已实现
✅ 面向对象设计（继承、多态、封装）
✅ 模板编程（Deck<T>）
✅ 运算符重载（算术、比较、流）
✅ 异常处理（自定义异常层次）
✅ 内存管理（RAII、深拷贝、移动语义）
✅ 文件IO（二进制格式保存/加载）
✅ 标准库使用（STL容器、算法）

### 待实现
⏳ 高级模板技术
⏳ 智能指针（unique_ptr, shared_ptr）
⏳ Lambda表达式
⏳ Qt信号槽机制
⏳ 多线程（可选）

## 编译和测试

### 编译成功 ✅
```bash
g++ -std=c++14 -I./include -o magic_trick_console \
    src/Card.cpp src/TwentyOneCardTrick.cpp src/main_console.cpp \
    -Wall -Wextra
```

### 可执行文件
- `magic_trick_console` (103KB)

### 测试状态
- ⏳ 基本功能测试（待完成）
- ⏳ 内存泄漏检测（valgrind）
- ⏳ 边界条件测试
- ⏳ 异常处理测试

## 下一步计划 (Next Steps)

### 立即任务
1. 测试控制台版本的基本功能
2. 修复任何发现的bug
3. 实现Leaderboard类
4. 实现Utils工具类（颜色和动画）
5. 添加27张牌魔术变体

### 短期任务（1-2天）
1. 完善控制台版本所有功能
2. 实现完整的计分系统
3. 添加配置系统
4. 完成Task 3的所有要求

### 中期任务（3-5天）
1. 准备GUI开发环境（安装Qt）
2. 设计GUI界面原型
3. 实现基本的Qt窗口
4. 准备卡牌图片资源

### 长期任务（6-10天）
1. 完整的GUI实现
2. 音效集成
3. 动画效果
4. 完整测试
5. 编写实训报告
6. 录制演示视频

## 文件清单 (File List)

### 头文件 (Headers)
- include/Exceptions.h (异常类)
- include/Card.h (卡牌类)
- include/Deck.h (牌堆模板类)
- include/MagicTrick.h (魔术基类)
- include/TwentyOneCardTrick.h (21张牌魔术)

### 源文件 (Sources)
- src/Card.cpp
- src/TwentyOneCardTrick.cpp
- src/main_console.cpp

### 配置文件 (Configuration)
- CMakeLists.txt
- Makefile
- .gitignore

### 文档 (Documentation)
- README.md
- 本文件 (PROGRESS.md)

### 目录结构
- include/ (头文件)
- src/ (源文件)
- assets/sounds/ (音效)
- assets/images/cards/ (卡牌图片)
- saves/ (存档)
- docs/ (文档)
- build/ (构建)

## 预计完成时间

- **Task 1 (65分)**: ✅ 已完成
- **Task 2 (80分)**: 预计2天
- **Task 3 (90分)**: 预计3-4天
- **Task 4 (100分)**: 预计5-7天

**总计**: 10-13天完成所有功能

## 备注 (Notes)

1. 代码使用英文命名，中文注释
2. 所有类都有完整的注释
3. 遵循C++14标准
4. 注重内存安全和异常处理
5. 代码风格统一，易于维护

---

**最后更新**: 2026-05-05
**开发者**: 软件工程专业学生
**项目状态**: 进行中 (Phase 1 完成)

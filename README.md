# 21张牌魔术

一个 C++14 课程实训项目，实现经典 21 张牌魔术，并扩展了 27 张/可配置牌数、保存加载、排行榜、回放、Qt GUI、音效和 socket 双人模式。

## 项目简介

21 张牌魔术的核心流程：

1. 从 21 张不同的牌中让观众记住一张。
2. 将牌从上到下依次发成 3 堆。
3. 观众告诉魔术师记住的牌在哪一堆。
4. 魔术师把观众选择的牌堆放到中间并合并。
5. 重复 3 轮后，目标牌会位于第 11 张。

本项目用 `MagicTrick` 抽象基类和多个派生类演示继承、多态、模板、运算符重载、异常处理和动态内存管理。

## 当前功能

- 21 张牌魔术经典流程
- 27 张牌魔术变体
- 15/21/27 张可配置三堆魔术
- 数字牌和标准扑克牌两种牌面
- 控制台彩色显示
- 逐张发牌动画
- 发牌、揭晓和结果提示音
- 观众互动模式
- 魔术师练习模式：系统模拟观众选堆，最后由魔术师手动选择揭晓牌
- `S` 保存并返回主菜单
- `P` 暂停、`R` 恢复
- 按玩家名保存/加载游戏进度
- 计分和前 10 名排行榜
- 回放记录和 HTML 回放报告导出
- Qt 图形界面，支持鼠标点击选择牌堆
- socket 双人魔术模式：一端扮演魔术师，一端扮演观众

## 编译运行

推荐使用增强版主入口：

```bash
make enhanced
./magic_trick_enhanced
```

常用命令：

```bash
make all          # 默认编译增强版
make enhanced     # 编译增强版主程序
make console      # 兼容命令，实际生成增强版入口
make test         # 编译测试程序
make run          # 编译并运行增强版
make run-test     # 编译并运行测试
make gui          # 编译 Qt GUI
make clean        # 清理构建产物
```

`magic_trick_console` 只是兼容旧命令的可执行文件，当前控制台主入口统一为增强版。

## 双人魔术模式

本机答辩演示可开两个终端。

终端 1：

```bash
./magic_trick_enhanced
```

选择：

```text
6 网络双人对战
1 我是魔术师
房间端口直接回车
```

终端 2：

```bash
./magic_trick_enhanced
```

选择：

```text
6 网络双人对战
2 我是观众
魔术师地址直接回车
房间端口直接回车
```

默认地址为 `127.0.0.1`，默认端口为 `5000`。如果端口被占用，魔术师端和观众端输入同一个新端口即可，例如 `5001`。

## 项目结构

```text
Cpp_traning/
├── include/
│   ├── Card.h
│   ├── Deck.h
│   ├── MagicTrick.h
│   ├── ThreePileCardTrick.h
│   ├── TwentyOneCardTrick.h
│   ├── TwentySevenCardTrick.h
│   ├── ConfigurableCardTrick.h
│   ├── NetworkGame.h
│   ├── ReplayManager.h
│   ├── Leaderboard.h
│   └── Exceptions.h
├── src/
│   ├── Card.cpp
│   ├── ThreePileCardTrick.cpp
│   ├── TwentyOneCardTrick.cpp
│   ├── TwentySevenCardTrick.cpp
│   ├── ConfigurableCardTrick.cpp
│   ├── NetworkGame.cpp
│   ├── ReplayManager.cpp
│   ├── Leaderboard.cpp
│   ├── Utils.cpp
│   ├── PileChoice.cpp
│   ├── main_enhanced.cpp
│   └── test_main.cpp
├── gui/
│   ├── qt_magic_trick.cpp
│   └── qt_magic_trick.pro
├── docs/
├── saves/
├── replays/
├── Makefile
├── CMakeLists.txt
└── README.md
```

## 类设计概览

- `Card`：卡牌实体，支持数字牌和标准扑克牌。
- `Deck<T>`：模板牌堆，使用动态数组，支持深拷贝、移动语义和运算符重载。
- `MagicTrick`：魔术抽象基类。
- `ThreePileCardTrick`：15/21/27 张三堆魔术的公共父类，承载发牌、整理、揭晓、保存加载等公共流程。
- `TwentyOneCardTrick`：固定 21 张牌，第 11 张揭晓。
- `TwentySevenCardTrick`：固定 27 张牌，第 14 张揭晓。
- `ConfigurableCardTrick`：支持 15/21/27 张配置。
- `NetworkGame`：socket 双人模式，魔术师端复用主游戏流程。
- `ReplayManager`：记录并导出回放。
- `Leaderboard`：保存排行榜。

## 测试

```bash
make -B enhanced
make -B console
make -B test
./test_program
git diff --check
```

当前单元测试覆盖 `Card`、`Deck`、异常处理、21 张牌基础流程和保存/加载。

## 答辩建议

推荐展示顺序：

1. `make enhanced && ./magic_trick_enhanced`
2. 普通观众模式
3. 魔术师练习模式
4. 保存/加载
5. 排行榜和回放
6. Qt GUI
7. 双人魔术模式

## 说明

本项目仅用于 C++ 课程实训和答辩演示。

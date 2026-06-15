# 快速开始指南

## 1. 编译

```bash
cd /Users/diamond/Desktop/Cpp_traning
make enhanced
```

`make console` 仍可使用，但现在只是兼容入口，实际等同于增强版。

## 2. 运行

```bash
./magic_trick_enhanced
```

或：

```bash
make run
```

## 3. 测试

```bash
make test
./test_program
```

## 4. Qt GUI

```bash
make gui
```

如果提示找不到 Qt，可先安装 Qt 并把 `qmake` 加入 `PATH`。

## 5. 双人魔术模式

开两个终端运行 `./magic_trick_enhanced`。

魔术师端选择：

```text
6 网络双人对战
1 我是魔术师
房间端口直接回车
```

观众端选择：

```text
6 网络双人对战
2 我是观众
魔术师地址直接回车
房间端口直接回车
```

默认本机地址为 `127.0.0.1`，默认端口为 `5000`。如果端口占用，两端输入同一个新端口即可。

## 常用命令

```bash
make enhanced
make console
make test
make run
make run-test
make gui
make clean
make help
```

## 项目状态

核心控制台增强版、Qt GUI、保存加载、排行榜、回放、音效和网络双人模式均已实现。答辩推荐使用 `magic_trick_enhanced` 作为主入口。

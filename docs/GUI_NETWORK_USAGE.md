# GUI and Network Usage

## Qt GUI

The GUI version is implemented with Qt Widgets in `gui/qt_magic_trick.cpp`.
It shows the 21-card trick in a window and lets the player click a pile with the mouse.

Build:

```bash
make gui
```

Run after build:

```bash
./build/qt_gui/magic_trick_gui.app/Contents/MacOS/magic_trick_gui
```

On macOS, install Qt with Homebrew:

```bash
brew install qt
export PATH="/opt/homebrew/opt/qt/bin:$PATH"
make gui
```

If you use an Intel Mac, the Qt path may be:

```bash
export PATH="/usr/local/opt/qt/bin:$PATH"
```

## Network Duel

The network mode is built into the enhanced console program:

```bash
make enhanced
./magic_trick_enhanced
```

Choose:

```text
6. 网络双人对战 (Network Duel)
```

One player chooses magician/server mode. The other chooses audience/client mode.

Same machine test:

```text
Magician host: port 19021
Audience host: 127.0.0.1, port 19021
```

Same LAN test:

1. The magician runs server mode and enters a port such as `19021`.
2. The audience enters the magician computer's LAN IPv4 address and the same port.
3. If the connection fails, allow the program through the macOS firewall or choose another port.

Protocol summary:

- Server sends the shuffled 21-card deck.
- Server sends three piles each round.
- Client sends the selected pile number.
- Server reorganizes the deck and reveals the 11th card after three rounds.

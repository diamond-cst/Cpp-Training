# 21 Card Magic Trick - C++ Training Project Plan

## Context
This is a C++ course training project that requires implementing a "21 Card Magic Trick" simulation. The magic trick works by:
1. Starting with 21 cards
2. Dealing them into 3 piles of 7 cards each
3. Asking the audience which pile contains their remembered card
4. Reorganizing by placing the chosen pile in the middle
5. Repeating this process 3 times
6. The remembered card will always end up at position 11 (index 10)

The project must demonstrate advanced C++ concepts including:
- Object-oriented design (inheritance, polymorphism)
- Operator overloading
- Templates
- Exception handling
- Memory management (constructors, destructors, deep copy)
- File I/O for save/load functionality

## Initial Exploration Phase

Starting exploration to understand:
1. Project structure requirements
2. Class hierarchy design
3. Required features for each task level (65%, 80%, 90%, 100%)
4. Best practices for C++ implementation

## Requirements Analysis

### Task 1 (65 points) - Basic Implementation
- Friendly console interface
- Random initialization of 21 cards (support numeric 1-21 or standard playing cards)
- Complete magic trick flow (3 rounds of dealing, asking, reorganizing)
- Basic operator overloading (<<, >>)
- Basic memory management with dynamic arrays

### Task 2 (80 points) - Enhanced Features
- Save/load game progress to file
- Multi-player save slots
- Pause/resume functionality (P/R keys)
- Add another magic trick variant (polymorphism demonstration)
- Enhanced operator overloading (+, *, ==)

### Task 3 (90 points) - Advanced Features
- Configurable parameters (deck size: 15, 27, etc.)
- Scoring system and leaderboard (top 10)
- Audience mode vs Magician practice mode
- Console colors for card suits
- Animation effects
- Comprehensive exception handling

### Task 4 (100 points) - Bonus Features
- GUI implementation (Qt/SFML/EasyX)
- Sound effects
- Network multiplayer
- Other innovations (replay, PDF export, etc.)

## Design Phase

### Class Architecture

```
MagicTrick (abstract base class)
├── TwentyOneCardTrick (21 cards, 3 piles, 3 rounds)
├── TwentySevenCardTrick (27 cards variant)
└── ForcedCardTrick (迫牌魔术 - forced card variant)

Card (represents a single card)
├── Supports numeric mode (1-21)
└── Supports standard playing cards (suit + rank)

Deck<T> (template class for card management)
├── Dynamic array with manual memory management
├── Deep copy support
└── Operator overloading (+, -, *, ==, !=, <<, >>)

GameState (serializable state)
SaveManager (handles file I/O for multiple save slots)

Custom Exceptions:
├── MagicTrickException (base)
├── InvalidCardException
├── OutOfBoundsException
├── FileIOException
├── InvalidGameStateException
└── InvalidInputException
```

### Core Algorithm for 21 Card Trick

1. **Initialization**: Create 21 unique cards
2. **Round Loop (3 times)**:
   - Deal cards into 3 piles (7 cards each) in round-robin fashion
   - Display the 3 piles
   - Ask which pile contains the remembered card (input: 1, 2, or 3)
   - Reorganize: place chosen pile in the middle, merge back into single deck
3. **Reveal**: Card at position 11 (index 10) is the remembered card

### File Structure

```
src/
├── main.cpp                    # Entry point, menu system
├── Card.h / Card.cpp          # Card class implementation
├── Deck.h / Deck.cpp          # Template Deck class
├── MagicTrick.h               # Abstract base class
├── TwentyOneCardTrick.h/cpp   # Main trick implementation
├── TwentySevenCardTrick.h/cpp # 27-card variant
├── ForcedCardTrick.h/cpp      # Forced card variant
├── GameState.h/cpp            # State serialization
├── SaveManager.h/cpp          # File I/O management
├── Exceptions.h               # Custom exception classes
├── Utils.h/cpp                # Helper functions (colors, animations)
└── Leaderboard.h/cpp          # Scoring and ranking system

saves/                          # Save game directory
├── slot_1.dat
├── slot_2.dat
└── ...

leaderboard.dat                 # High scores file
```

### Key Implementation Details

#### 1. Card Class
- Two constructors: `Card(int value)` for numeric, `Card(Suit, Rank)` for standard
- Operator overloading: ==, !=, <, >, <<, >>
- `toString()` method for display with optional color support

#### 2. Deck Template Class
- Dynamic array: `T* cards`
- Capacity management with resize strategy (double when full)
- Copy constructor for deep copy
- Move constructor/assignment for efficiency
- Operators: +, -, *, ==, !=, [], <<, >>

#### 3. TwentyOneCardTrick Class
- Member variables:
  - `Deck<Card> workingDeck`
  - `int currentRound` (0-3)
  - `Card* rememberedCard` (for verification)
  - `std::string playerName`
  - `int score`
- Methods:
  - `dealIntoPiles()` - creates 3 piles of 7 cards
  - `reorganizePiles(int chosenPile)` - merges with chosen in middle
  - `revealCard()` - shows position 11
  - `verifyGuess()` - checks if correct

#### 4. Save/Load System
- Binary format for efficiency
- Structure: playerName, trickType, currentRound, cardOrder[], timestamp
- Multiple slots (5 slots for different players)
- Auto-save on pause

#### 5. Scoring System (Task 3)
- Audience mode: +10 correct, -5 wrong, track streak
- Magician mode: time-based scoring
- Leaderboard: top 10 players with name, score, date

#### 6. Console Enhancements
- ANSI color codes for suits (red: ♥♦, black: ♠♣)
- Animation: delay between card displays
- Clear screen between rounds
- Formatted output with borders

### Implementation Plan

#### Phase 1: Core Classes (Task 1 - 65 points)
1. Implement `Card` class with basic operators
2. Implement `Deck<T>` template with memory management
3. Create `MagicTrick` abstract base class
4. Implement `TwentyOneCardTrick` with complete algorithm
5. Create basic console UI with menu
6. Test the magic trick flow end-to-end

**Critical Files**:
- `Card.h`, `Card.cpp`
- `Deck.h` (template, header-only or with .tpp)
- `MagicTrick.h`
- `TwentyOneCardTrick.h`, `TwentyOneCardTrick.cpp`
- `main.cpp`

#### Phase 2: Save/Load & Polymorphism (Task 2 - 80 points)
1. Implement `GameState` serialization
2. Create `SaveManager` for multi-slot saves
3. [x] Add pause/resume functionality (P/R keys)
   - P pauses during pile selection and R resumes.
4. Implement second magic trick variant (e.g., `TwentySevenCardTrick`)
5. Enhance operator overloading (+, *, ==)
6. Test save/load with different game states
7. [x] Support player-name save/load files
   - Enhanced menu saves and loads `saves/player_<name>.dat`.

**Critical Files**:
- `GameState.h`, `GameState.cpp`
- `SaveManager.h`, `SaveManager.cpp`
- `TwentySevenCardTrick.h`, `TwentySevenCardTrick.cpp`
- Update `main.cpp` for trick selection

#### Phase 3: Advanced Features (Task 3 - 90 points)
1. [x] Add configuration system (deck size, display mode)
   - Added `ConfigurableCardTrick` with 15/21/27 card support.
   - Added color, animation, and mode options in the enhanced menu.
   - Added hidden-card display mode and numeric-card mode.
2. [x] Implement scoring system
   - Audience mode: +10 / -5.
   - Magician practice mode: time-based score / -5 with practice hints.
   - Leaderboard streak now updates and resets from actual game results.
3. [x] Create `Leaderboard` class with file persistence
   - Leaderboard updates after new games and loaded games are completed.
4. [x] Add console colors using ANSI codes
   - Card suit colors and styled menus are available through `Utils`.
5. [x] Implement animation effects
   - Animation can be enabled or disabled before a new game.
   - Piles are displayed card by card when animation is enabled.
6. [x] Add comprehensive exception handling
   - Save files now carry trick type metadata and reject mismatched/corrupt loads.
   - Main enhanced flow catches `std::bad_alloc` and `std::ios_base::failure`.
7. [x] Create audience mode vs magician practice mode

**Critical Files**:
- `Leaderboard.h`, `Leaderboard.cpp`
- `Utils.h`, `Utils.cpp` (colors, animations)
- `Exceptions.h`
- Update all classes with exception handling

#### Phase 4: Bonus Features (Task 4 - 100 points)
Optional enhancements:
- [x] GUI using Qt or SFML
  - Added a Qt Widgets GUI in `gui/qt_magic_trick.cpp`.
  - Added `make gui` and CMake `BUILD_GUI` support.
  - GUI supports mouse click pile selection and reveal after three rounds.
- [x] Sound effects
  - Added optional terminal beep feedback for dealing and reveal results.
- [x] Network multiplayer
  - Added socket-based two-player duel.
  - Magician runs the server; audience connects as a client and submits pile choices.
- [x] Replay system
  - Added `ReplayManager` with automatic per-game `.txt` replay recording.
  - Added main-menu replay browser.
- [x] Report export
  - Added automatic HTML replay report export for completed games.

### Verification Strategy

1. **Unit Testing**:
   - Test Card comparison operators
   - Test Deck memory management (no leaks with valgrind)
   - Test operator overloading results
   - Test save/load round-trip

2. **Integration Testing**:
   - Run complete 21-card trick multiple times
   - Verify card always ends at position 11
   - Test pause/resume mid-game
   - Test multiple save slots

3. **User Acceptance Testing**:
   - Friendly UI with clear instructions
   - Error handling for invalid inputs
   - Smooth animation and colors
   - Leaderboard persistence

### Deliverables

1. **Source Code**: All .h and .cpp files with Chinese comments
2. **Build System**: 
   - CMakeLists.txt for cross-platform build
   - Support both console and GUI versions
3. **Assets** (for GUI):
   - Card images (52 cards + back design)
   - Sound effects (.wav or .mp3 files)
   - Icon and resources
4. **Documentation**:
   - README.md: Compilation and usage instructions (中英文)
   - INSTALL.md: Dependency installation guide
5. **Report** (实训报告):
   - 需求分析 (Requirements analysis)
   - 原型图 (UI mockups/wireframes)
   - 类逻辑关系图 (UML class diagram)
   - 程序主要流程图 (Main flowchart)
   - 程序演示视频 (Demo video showing all features)

## Final Implementation Plan

### Step 1: Project Setup
- Create directory structure (src/, include/, assets/, saves/, docs/)
- Set up CMakeLists.txt with Qt or SFML
- Create .gitignore for build artifacts

### Step 2: Core Classes (Console Version)
**Files to create**:
- `include/Exceptions.h` - Custom exception hierarchy
- `include/Card.h`, `src/Card.cpp` - Card class with suit/rank
- `include/Deck.h` - Template Deck class (header-only or with .tpp)
- `include/MagicTrick.h` - Abstract base class
- `include/TwentyOneCardTrick.h`, `src/TwentyOneCardTrick.cpp` - Main trick
- `src/main_console.cpp` - Console version entry point

**Key implementations**:
- Card: enum for Suit (♠♥♣♦) and Rank (A-K)
- Deck: dynamic array with copy/move constructors
- TwentyOneCardTrick: complete algorithm with 3 rounds
- Operator overloading: <<, >>, +, -, *, ==, !=, <, >

### Step 3: Save/Load & Polymorphism
**Files to create**:
- `include/GameState.h`, `src/GameState.cpp` - Serialization
- `include/SaveManager.h`, `src/SaveManager.cpp` - File I/O
- `include/TwentySevenCardTrick.h`, `src/TwentySevenCardTrick.cpp` - Variant
- `include/ForcedCardTrick.h`, `src/ForcedCardTrick.cpp` - Another variant

**Key implementations**:
- Binary file format for game state
- Multiple save slots (5 slots)
- Pause/resume with P/R keys
- Polymorphic trick selection via base class pointer

### Step 4: Scoring & Advanced Console Features
**Files to create**:
- `include/Leaderboard.h`, `src/Leaderboard.cpp` - Top 10 scores
- `include/Utils.h`, `src/Utils.cpp` - ANSI colors, animations
- `include/Config.h`, `src/Config.cpp` - Game settings

**Key implementations**:
- Scoring: +10 correct, -5 wrong, streak tracking
- ANSI color codes for red/black suits
- Animation: delay between card displays
- Configurable deck size (15, 21, 27 cards)
- Audience mode vs Magician practice mode

### Step 5: GUI Implementation (Qt)
**Files to create**:
- `include/MainWindow.h`, `src/MainWindow.cpp` - Main GUI window
- `include/CardWidget.h`, `src/CardWidget.cpp` - Card display widget
- `include/GameWidget.h`, `src/GameWidget.cpp` - Game area
- `include/LeaderboardDialog.h`, `src/LeaderboardDialog.cpp` - Score window
- `include/SaveLoadDialog.h`, `src/SaveLoadDialog.cpp` - Save/load UI
- `src/main_gui.cpp` - GUI version entry point
- `resources.qrc` - Qt resource file for images/sounds

**Key implementations**:
- QMainWindow with menu bar (New Game, Load, Save, Leaderboard, Exit)
- Card images displayed as QLabel or custom QWidget
- Mouse click events for pile selection
- QPropertyAnimation for card dealing animation
- QSound or QMediaPlayer for sound effects
- QTableWidget for leaderboard display

### Step 6: Sound Effects & Polish
**Assets to add**:
- `assets/sounds/deal.wav` - Card dealing sound
- `assets/sounds/flip.wav` - Card flip sound
- `assets/sounds/reveal.wav` - Final reveal sound
- `assets/sounds/correct.wav` - Correct guess
- `assets/sounds/wrong.wav` - Wrong guess
- `assets/images/cards/` - 52 card images + back

**Key implementations**:
- Sound playback on events
- Volume control in settings
- Smooth animations (fade in/out, slide)
- Polished UI with consistent styling

### Step 7: Testing & Documentation
**Testing checklist**:
- ✓ Memory leak check with valgrind
- ✓ All operator overloads work correctly
- ✓ Save/load preserves exact game state
- ✓ Magic trick algorithm always works (card at position 11)
- ✓ Exception handling for all error cases
- ✓ GUI responsive and intuitive
- ✓ Sound effects play correctly
- ✓ Leaderboard persists across sessions

**Documentation to write**:
- Code comments in Chinese for all classes/methods
- README with build instructions
- Report with diagrams and screenshots
- Record demo video (5-10 minutes)

### Verification Strategy

**Console Version Testing**:
```bash
# Compile and run
mkdir build && cd build
cmake .. -DBUILD_CONSOLE=ON
make
./magic_trick_console

# Test scenarios:
# 1. Complete a full 21-card trick
# 2. Save game at round 2, exit, reload
# 3. Try invalid inputs (letters, out of range)
# 4. Test all three trick variants
# 5. Check leaderboard persistence
```

**GUI Version Testing**:
```bash
# Compile with Qt
cmake .. -DBUILD_GUI=ON
make
./magic_trick_gui

# Test scenarios:
# 1. Click through complete game
# 2. Test all menu options
# 3. Verify animations smooth
# 4. Check sound effects play
# 5. Test save/load dialogs
# 6. Verify leaderboard window
```

**Memory Testing**:
```bash
# Check for memory leaks
valgrind --leak-check=full ./magic_trick_console
valgrind --leak-check=full ./magic_trick_gui
```

### Timeline Estimate

- **Step 1-2** (Core classes): 2-3 days
- **Step 3** (Save/load): 1-2 days  
- **Step 4** (Scoring/console): 1-2 days
- **Step 5** (GUI): 3-4 days
- **Step 6** (Sound/polish): 1-2 days
- **Step 7** (Testing/docs): 2-3 days

**Total**: 10-16 days for complete implementation

### Critical Success Factors

1. **Start with console version** - Ensure core algorithm works before GUI
2. **Test incrementally** - Verify each feature before moving to next
3. **Memory management** - Use valgrind frequently to catch leaks early
4. **Qt documentation** - Reference Qt docs for GUI components
5. **Asset preparation** - Find or create card images and sounds early
6. **Report alongside code** - Document as you build, not at the end

### Development Approach

**Target**: Task 4 (100 points) - Full implementation with GUI

**User Preferences**:
- Card type: Standard playing cards (♠♥♣♦)
- Code style: English code with Chinese comments
- Target score: 100 points (includes GUI)

**Order of Implementation**:
1. **Phase 1**: Task 1 foundation (core classes, console version)
2. **Phase 2**: Task 2 features (save/load, polymorphism)
3. **Phase 3**: Task 3 features (scoring, colors, animations)
4. **Phase 4**: Task 4 features (GUI with Qt or SFML, sound effects)

**GUI Technology Choice**:
- **Recommended**: Qt (cross-platform, mature, good documentation)
- **Alternative**: SFML (lighter, game-focused)
- **Fallback**: EasyX (Windows-only, simpler but limited)

**Code Quality Focus**:
- English variable/function names
- Chinese comments for clarity (中文注释)
- Proper indentation and formatting
- Memory leak prevention (test with valgrind on macOS)
- Exception safety
- Const correctness
- Documentation for all public methods

**GUI Features to Implement**:
1. Main menu with trick selection
2. Card display with images (♠♥♣♦)
3. Mouse click to select pile (1, 2, or 3)
4. Animated card dealing
5. Score display and leaderboard window
6. Save/load dialog
7. Sound effects for card dealing and reveal

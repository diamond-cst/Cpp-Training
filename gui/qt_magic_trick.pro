QT += widgets network
CONFIG += c++14

macx:LIBS += -framework AudioToolbox -framework CoreFoundation

TARGET = magic_trick_gui
TEMPLATE = app

INCLUDEPATH += ../include

HEADERS += \
    GuiUtils.h \
    CardWidgets.h \
    MagicTrickWindow.h

SOURCES += main.cpp \
    GuiUtils.cpp \
    CardWidgets.cpp \
    MagicTrickWindow.cpp \
    LocalGameController.cpp \
    InfoPages.cpp \
    NetworkGamePage.cpp \
    ../src/Card.cpp \
    ../src/ThreePileCardTrick.cpp \
    ../src/TwentyOneCardTrick.cpp \
    ../src/TwentySevenCardTrick.cpp \
    ../src/ConfigurableCardTrick.cpp \
    ../src/Utils.cpp \
    ../src/PileChoice.cpp \
    ../src/ReplayManager.cpp \
    ../src/Leaderboard.cpp

RESOURCES += resources.qrc

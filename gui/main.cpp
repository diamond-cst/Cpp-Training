#include "MagicTrickWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MagicTrickWindow window;
    window.show();
    return app.exec();
}

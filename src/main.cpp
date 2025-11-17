#include <QApplication>

#include "mainpanel.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    auto main_panel = std::make_unique<MainPanel>();

    return QApplication::exec();
}

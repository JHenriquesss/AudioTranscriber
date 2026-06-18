#include "MainWindow.hpp"

#include <QApplication>
#include <QCoreApplication>

#include <filesystem>

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);
    MainWindow window;
    window.show();
    return application.exec();
}

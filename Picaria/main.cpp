#include "Picaria.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Picaria picaria;

    picaria.show();

    return a.exec();
}

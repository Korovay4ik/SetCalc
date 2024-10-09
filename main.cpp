#include "mysetscalc.h"
#include <cmath>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MySetsCalc w;
    w.setWindowTitle("Калькулятор");
    w.show();
    return a.exec();
}

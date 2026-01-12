#include <QApplication>

#include "logutils.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  LOGUTILS::initLogging();

  MainWindow w;
  w.show();

  return a.exec();
}

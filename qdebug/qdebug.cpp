#include "qlogging.h"
// #define QT_MESSAGELOGCONTEXT 1
#include "qdebug.h"
// #include "qlogging.h"
#include <cstdio>
#include <QCoreApplication>

int main(int argc, char **argv) {
  qSetMessagePattern("[%{file} %{function}:%{line}] %{message}");
  QCoreApplication a(argc, argv);
  qDebug() << "debug message";
  qInfo() << "info message";
  qWarning() << "warning message";
  qCritical() << "error message";
  fprintf(stderr, "error message\n");

}

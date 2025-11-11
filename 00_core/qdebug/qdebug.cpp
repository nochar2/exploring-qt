#include "qlogging.h"
#include "qdebug.h"
#include <cstdio>
#include <QCoreApplication>

int main(int argc, char **argv) {
  qSetMessagePattern("[%{file} %{function}:%{line}] %{message}");
  QCoreApplication a(argc, argv);
  qDebug()    << "debug message";
  qInfo()     << "info message";
  qWarning()  << "warning message";
  qCritical() << "error message";
  fprintf(stderr, "error message from fprintf(stderr, ...)\n");
}

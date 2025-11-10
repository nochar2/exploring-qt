#include "qgraphicseffect.h"
#include <QPushButton>
#include <QApplication>
#include <QGraphicsOpacityEffect>

int main(int argc, char **argv) {
   QApplication app(argc, argv);

   QPushButton btn;
   btn.setText("aaaaaaaaaaaaaaaaaaaaaa");

   QGraphicsOpacityEffect eff(&btn);
   eff.setOpacity(0);


   btn.show();
   return app.exec();
}

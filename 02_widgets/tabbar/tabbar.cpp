#include <QApplication>
#include <QTabBar>
#include <QTabWidget>
#include <QLabel>

int main(int argc, char **argv) {
  QApplication app(argc , argv);

  // QTabBar tabbar;
  // tabbar.setTabsClosable(true);
  // tabbar.setExpanding(false);
  // tabbar.setMovable(true);
  // tabbar.addTab("My chart!");
  // tabbar.addTab("My other chart!");
  // tabbar.addTab("My yet another chart!");

  QTabWidget tw;
  tw.setMovable(true);
  
  QLabel label1; label1.setText("First chart.");
  QLabel label2; label2.setText("Second chart.");
  QLabel label3; label3.setText("Third chart.");

  tw.addTab(&label1, "First chart");
  tw.addTab(&label2, "Second chart");
  tw.addTab(&label3, "Third chart");

  tw.show();
  return app.exec();
}

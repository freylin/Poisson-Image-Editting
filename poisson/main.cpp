#include "poisson.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    poisson w;
    QPalette palette;  //创建一个调色板的对象
    QPixmap pixmap(":imgs/back.jpg");
    palette.setBrush(w.backgroundRole(),QBrush(pixmap));

    w.setPalette(palette);

    w.show();

    return a.exec();
}

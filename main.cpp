#include <QtGui/QApplication>
#include "QStyle"

#include "frg_shader_author.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPalette p(QColor(40, 40, 40));
    p.setColor(QPalette::Text, QColor(0, 0, 0));
    p.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    p.setColor(QPalette::Button, QColor(50, 50, 50));
    p.setColor(QPalette::WindowText, QColor(255, 255, 255, 180));
    p.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
    a.setPalette(p);
    frg_Shader_Author *w = new frg_Shader_Author;
    w->show();
    return a.exec();
}

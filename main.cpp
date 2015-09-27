#include "widget.h"
#include <QApplication>
#include "playThread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();

    playThread pt;
    pt.start(QThread::HighPriority);
    return a.exec();
}

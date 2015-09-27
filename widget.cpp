#include "widget.h"
#include "ui_widget.h"
#include <QPixmap>
#include <QPalette>
#include <QFontDatabase>
#include <QDebug>
#include <QString>
#include <QFont>
#include <QLabel>
#include <QPalette>

#define MAIN_H  1270
#define MAIN_W  720
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    /* set the background size */
    this->resize(QSize(MAIN_H, MAIN_W));
    this->setAutoFillBackground(true);

    /* set background image */
    QPixmap pixmap(":/image/background.jpg");
    QPixmap fitpixmap=pixmap.scaled(MAIN_H, MAIN_W, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPalette palette;
    palette.setBrush(QPalette::Background, QBrush(fitpixmap));
    this->setPalette(palette);

    QFont serifFont("SimHei", 15, QFont::Bold);
//    ui->nextStation->setFont(serifFont);
//    QPalette p = ui->nextStation->palette();
//    p.setColor(QPalette::WindowText, Qt::white);
//    ui->nextStation->setPalette(p);
//    ui->nextStation->setGeometry(40, 40, 50, 200);
//    ui->nextStation->setText("美国");

//    foreach (const QString &family, database.families(QFontDatabase::SimplifiedChinese))
//        qDebug()<< " --:" << family;
}

Widget::~Widget()
{
    delete ui;
}

#include "widget.h"
#include "ui_widget.h"
#include "scintilla/qt/ScintillaEdit/ScintillaEdit.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);


    auto edit = new ScintillaEdit();

    QColor bgColor = QColor("#555AF0");
    edit->send(SCI_STYLESETBACK, STYLE_DEFAULT, bgColor.red() | bgColor.green() << 8 | bgColor.blue() << 16);  // 深灰色背景
    edit->send(SCI_SETMARGINWIDTHN, 0, 20);

    ui->widget->layout()->addWidget(edit);
}

Widget::~Widget()
{
    delete ui;
}

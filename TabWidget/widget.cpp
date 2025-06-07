#include "widget.h"
#include "./ui_widget.h"

#include <QLabel>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    QString title = ui->lineEdit->text();

    QWidget* w = new QWidget;
    QLabel* l = new QLabel(title,w);
    // QVBoxLayout* layout = new QVBoxLayout(w);
    // layout->addWidget(new QLabel(title));

    ui->widget->addPage(w, title, ui->checkBox->isChecked());

    int num = title.toInt();
    num++;
    ui->lineEdit->setText(QString::number(num));
}


void Widget::on_pushButton_2_clicked()
{
    ui->widget->removeAll();
    ui->lineEdit->setText(QString::number(1));
}

void Widget::on_pushButton_3_clicked()
{
    // ui->widget->print();
}


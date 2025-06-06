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

    ui->widget->addTab(w, title, ui->checkBox->isChecked());

    int num = title.toInt();
    num++;
    ui->lineEdit->setText(QString::number(num));
}


void Widget::on_pushButton_2_clicked()
{

}


void Widget::on_pushButton_3_clicked()
{
    // QString title = ui->lineEdit_2->text();

    // QWidget* w = new QWidget;
    // QVBoxLayout* layout = new QVBoxLayout(w);
    // layout->addWidget(new QLabel(title));

    // TabContainer* c = new TabContainer;

    // ui->widget->addContainer(c);
    // c->addTab(w, title);
}


#include "widget.h"
#include "./ui_widget.h"
#include "loadingwidget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    pLoading = new LoadingWidget(this);
    pPictureLoading = new PictureLoadingWidget(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_4_clicked()
{
    pPictureLoading->startLoading(ui->pushButton_3, true);
    pPictureLoading->startLoading(ui->toolButton, true);
    pPictureLoading->startLoading(ui->radioButton, true);
    pPictureLoading->startLoading(ui->checkBox, true);
}


void Widget::on_pushButton_5_clicked()
{
    pPictureLoading->stopLoading(ui->pushButton_3);
    pPictureLoading->stopLoading(ui->toolButton);
    pPictureLoading->stopLoading(ui->radioButton);
    pPictureLoading->stopLoading(ui->checkBox);
}


void Widget::on_pushButton_7_clicked()
{
    pLoading->startLoading(ui->comboBox, false, 20);
}


void Widget::on_pushButton_8_clicked()
{
    pLoading->stopLoading(ui->comboBox);
}


void Widget::on_pushButton_9_clicked()
{
    pLoading->startLoading(ui->label, true);
}


void Widget::on_pushButton_10_clicked()
{
    pLoading->stopLoading(ui->label);
}


void Widget::on_pushButton_11_clicked()
{
    pPictureLoading->startLoading(ui->listWidget->viewport());
}


void Widget::on_pushButton_12_clicked()
{
    pPictureLoading->stopLoading(ui->listWidget->viewport());
}


void Widget::on_pushButton_13_clicked()
{
    pLoading->startLoading(ui->tableWidget->viewport(), false, 50);
}


void Widget::on_pushButton_14_clicked()
{
    pLoading->stopLoading(ui->tableWidget->viewport());
}


void Widget::on_pushButton_15_clicked()
{
    pPictureLoading->startLoading(ui->widget);
    pLoading->startLoading(ui->groupBox);
    pLoading->startLoading(ui->tabWidget, true, 20);
}


void Widget::on_pushButton_16_clicked()
{
    pPictureLoading->stopLoading(ui->widget);
    pLoading->stopLoading(ui->groupBox);
    pLoading->stopLoading(ui->tabWidget);
}


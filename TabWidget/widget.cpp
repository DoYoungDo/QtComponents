#include "widget.h"
#include "./ui_widget.h"

#include <QLabel>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->widget->setTabsClosable(true);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    QString title = ui->lineEdit->text();
    if(allWidgets.contains(title))
    {
        QMessageBox::information(this, "", "title: " + title + "已存在");
        return;
    }

    QWidget* w = new QWidget;
    QLabel* l = new QLabel(title,w);

    allWidgets.insert(title, w);

    ui->widget->addPage(w, title, ui->checkBox->isChecked());


    int num = title.toInt();
    num++;
    ui->lineEdit->setText(QString::number(num));
}


void Widget::on_pushButton_2_clicked()
{
    ui->widget->removeAll();
    ui->lineEdit->setText(QString::number(1));
    allWidgets.clear();
}

void Widget::on_pushButton_3_clicked()
{
    // ui->widget->print();
}


void Widget::on_pushButton_4_clicked()
{
    QString baseTitle = ui->lineEdit_2->text();
    if(!allWidgets.contains(baseTitle))
    {
        QWidget* w = new QWidget;
        QLabel* l = new QLabel(baseTitle,w);

        allWidgets.insert(baseTitle, w);

        ui->widget->addPage(w, baseTitle);

        QMessageBox::information(this, "", "title: " + baseTitle + "不存在，默认添加");
        return;
    }

    QString title = ui->lineEdit_3->text();
    if(allWidgets.contains(title))
    {
        QMessageBox::information(this, "", "title: " + title + "已经存在");
        return;
    }

    QWidget* w = new QWidget;
    QLabel* l = new QLabel(title,w);

    allWidgets.insert(title, w);

    TabWidget::Orientations ori = (TabWidget::Orientations)(ui->radioButton_3->isChecked() ? 0 :
                                                                (ui->radioButton->isChecked() ? 1 :
                                                                     (ui->radioButton_4->isChecked() ? 2 :
                                                                          (ui->radioButton_2->isChecked() ? 3 :
                                                                               4))));

    ui->widget->addPage(w, title, allWidgets.value(baseTitle), ori);
}


void Widget::on_widget_pageCloseRequested(QWidget *w, const QString &)
{
    qDebug() << "on page close request";
    ui->widget->removePage(w);
}


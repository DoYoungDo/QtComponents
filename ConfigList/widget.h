#ifndef WIDGET_H
#define WIDGET_H

#include "configurationdiaplayer.h"
#include "highlighter.h"

#include <QWidget>
class ConfigurationDiaplayer;
struct SchemaProperty;

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_right_propertyValueChanged(const SchemaProperty &, const QString &, const QString &);

    void on_pushButton_6_clicked();

    void on_pushButton_8_clicked();

    void on_checkBox_clicked(bool checked);

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_16_clicked();

    void on_pushButton_17_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_18_clicked();

private:
    Ui::Widget *ui;
    QList<SchemaConfiguration> mDefaultConfigs;
    QssHighlighter* qssHighlighter;
    JsonHighlighter* jsonHighlighter;
    CppHighlighter* cppHighlighter;
    CppHighlighter* cppHighlighter1;
};
#endif // WIDGET_H

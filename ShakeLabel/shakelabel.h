#ifndef SHAKELABEL_H
#define SHAKELABEL_H

#include <QLabel>

class ShakeLabelPrivate;
class ShakeLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ShakeLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    explicit ShakeLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    ~ShakeLabel();

public slots:
    void shake();

protected:
    explicit ShakeLabel(ShakeLabelPrivate&d, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    QScopedPointer<ShakeLabelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ShakeLabel)
};

#endif // SHAKELABEL_H

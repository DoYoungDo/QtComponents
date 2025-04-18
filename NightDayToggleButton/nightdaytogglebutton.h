#ifndef NIGHTDAYTOGGLEBUTTON_H
#define NIGHTDAYTOGGLEBUTTON_H

#include <QWidget>

class NightDayToggleButtonPrivate;
class NightDayToggleButton : public QWidget
{
    Q_OBJECT
public:
    explicit NightDayToggleButton(QWidget *parent = nullptr);

    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void timerEvent(QTimerEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    NightDayToggleButtonPrivate* _d;
    friend class NightDayToggleButtonPrivate;
};

#endif // NIGHTDAYTOGGLEBUTTON_H

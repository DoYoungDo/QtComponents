#include "nightdaytogglebutton.h"

#include <QPainter>
#include <QPainterPath>

class NightDayToggleButtonPrivate{
    NightDayToggleButtonPrivate(NightDayToggleButton* q)
        :_q(q)
    {}

    bool mIsPressing = false;

    NightDayToggleButton* _q;
    friend class NightDayToggleButton;
};

NightDayToggleButton::NightDayToggleButton(QWidget *parent)
    : QWidget{parent}
    , _d(new NightDayToggleButtonPrivate(this))
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

void NightDayToggleButton::mousePressEvent(QMouseEvent* event)
{
    _d->mIsPressing = true;
}

void NightDayToggleButton::mouseReleaseEvent(QMouseEvent* event)
{
    _d->mIsPressing = false;
}

void NightDayToggleButton::mouseMoveEvent(QMouseEvent* event)
{

}

void NightDayToggleButton::timerEvent(QTimerEvent* event)
{

}

void NightDayToggleButton::paintEvent(QPaintEvent* event)
{
    QRectF thisRect= this->rect();
    QRectF roundrect = thisRect.marginsRemoved(QMargins(2,2,2,2));
    QRectF sunOrMoonRect = QRectF(roundrect.left() + 20, roundrect.top() + 20, roundrect.height() - 40, roundrect.height() - 40);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(Qt::red);
    p.setPen(Qt::transparent);

    QPainterPath rectPath;
    rectPath.addRect(thisRect);
    QPainterPath roundRectPath;
    roundRectPath.addRoundedRect(roundrect, roundrect.height() / 2 - 4, roundrect.height() / 2 - 4);
    QPainterPath sunOrMoonPath;
    sunOrMoonPath.addEllipse(sunOrMoonRect);

    QPainterPath shadowPath;
    shadowPath.addEllipse(sunOrMoonPath.boundingRect().center(), sunOrMoonRect.height() + sunOrMoonRect.height() / 2, sunOrMoonRect.height() + sunOrMoonRect.height() / 2);
    QPainterPath shadowPath1;
    shadowPath1.addEllipse(sunOrMoonPath.boundingRect().center(), sunOrMoonRect.height() + (sunOrMoonRect.height() / 2) * 3, sunOrMoonRect.height() + (sunOrMoonRect.height() / 2) * 3);
    QPainterPath shadowPath2;
    shadowPath2.addEllipse(sunOrMoonPath.boundingRect().center(), sunOrMoonRect.height() + (sunOrMoonRect.height() / 2) * 4, sunOrMoonRect.height() + (sunOrMoonRect.height() / 2) * 4);


    p.save();
    p.setBrush(QColor(0x4874ab));
    p.drawPath(roundRectPath);
    p.restore();

    p.save();
    p.setOpacity(0.5);
    p.setBrush(Qt::gray);
    p.drawPath(roundRectPath - (roundRectPath - shadowPath2));
    p.drawPath(roundRectPath - (roundRectPath - shadowPath1));
    p.drawPath(roundRectPath - (roundRectPath - shadowPath));
    p.restore();

    p.save();
    p.setBrush(Qt::yellow);
    p.drawPath(sunOrMoonPath);
    p.restore();

    // p.drawPath(rectPath - roundRectPath);
    // p.drawRoundedRect(rect, rect.height() / 2 - 2, rect.height() / 2 - 2);
}

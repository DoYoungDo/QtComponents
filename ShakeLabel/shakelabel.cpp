#include "shakelabel.h"

#include <QAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>


class ShakeLabelPrivate{
    ShakeLabelPrivate(ShakeLabel*q)
        :q_ptr(q){}
    void initAnimation(){
        Q_Q(ShakeLabel);

        QPointF rowPos = q->pos();
        QPointF nowPos = rowPos;
        auto createAnimation = [&](int duration,  QPointF start, QPointF end){
            qDebug() << start << end;
            QPropertyAnimation * animation = new QPropertyAnimation(q, "pos", q);
            animation->setDuration(duration);
            animation->setStartValue(start);
            animation->setEndValue(end);
            return animation;
        };

        QSequentialAnimationGroup* group = new QSequentialAnimationGroup(q);
        group->addAnimation(createAnimation(50, nowPos, nowPos = QPointF(nowPos.x() + 50, nowPos.y())));
        group->addAnimation(createAnimation(150, nowPos, nowPos = QPointF(nowPos.x() + 50, nowPos.y())));
        group->addAnimation(createAnimation(100, nowPos, nowPos = QPointF(nowPos.x() - 100, nowPos.y())));
        group->addAnimation(createAnimation(100, nowPos, nowPos = QPointF(nowPos.x() + 100, nowPos.y())));
        group->addAnimation(createAnimation(100, nowPos, nowPos = QPointF(nowPos.x() - 100, nowPos.y())));
        group->addAnimation(createAnimation(100, nowPos, nowPos = QPointF(nowPos.x() + 100, nowPos.y())));
        group->addAnimation(createAnimation(100, nowPos, nowPos = QPointF(nowPos.x() - 100, nowPos.y())));

        pAnimationGroup = group;
    }
    ShakeLabel* q_ptr;
    Q_DECLARE_PUBLIC(ShakeLabel)

    QAnimationGroup* pAnimationGroup = nullptr;
};

ShakeLabel::ShakeLabel(QWidget* parent, Qt::WindowFlags f)
    :QLabel(parent, f)
    ,d_ptr(new ShakeLabelPrivate(this))
{

}

ShakeLabel::ShakeLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
    :QLabel(text, parent, f)
    ,d_ptr(new ShakeLabelPrivate(this))
{

}

ShakeLabel::~ShakeLabel()
{

}

void ShakeLabel::shake()
{
    Q_D(ShakeLabel);

    if(d->pAnimationGroup)
    {
        if(d->pAnimationGroup->state() != QAnimationGroup::Stopped)
        {
            return;
        }

        delete d->pAnimationGroup;
    }

    d->initAnimation();
    d->pAnimationGroup->start();
}

ShakeLabel::ShakeLabel(ShakeLabelPrivate& d, QWidget* parent, Qt::WindowFlags f)
    :QLabel(parent, f)
    ,d_ptr(&d)
{

}

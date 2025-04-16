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
        QPropertyAnimation * a1 = new QPropertyAnimation(q, "pos", q);
        a1->setDuration(50);
        a1->setStartValue(rowPos);
        a1->setEndValue(nowPos = QPointF(nowPos.x() + 50, nowPos.y()));

        QPropertyAnimation * a2 = new QPropertyAnimation(q, "pos", q);
        a2->setDuration(80);
        a2->setStartValue(nowPos);
        a2->setEndValue(nowPos = QPointF(nowPos.x() + 50, nowPos.y()));

        QPropertyAnimation * a3 = new QPropertyAnimation(q, "pos", q);
        a3->setDuration(100);
        a3->setStartValue(nowPos);
        a3->setEndValue(nowPos = QPointF(nowPos.x() - 100, nowPos.y()));

        QPropertyAnimation * a4 = new QPropertyAnimation(q, "pos", q);
        a4->setDuration(100);
        a4->setStartValue(nowPos);
        a4->setEndValue(nowPos = QPointF(nowPos.x() + 100, nowPos.y()));

        QPropertyAnimation * a5 = new QPropertyAnimation(q, "pos", q);
        a5->setDuration(100);
        a5->setStartValue(nowPos);
        a5->setEndValue(nowPos = QPointF(nowPos.x() - 100, nowPos.y()));

        QPropertyAnimation * a6 = new QPropertyAnimation(q, "pos", q);
        a6->setDuration(100);
        a6->setStartValue(nowPos);
        a6->setEndValue(nowPos = QPointF(nowPos.x() + 100, nowPos.y()));

        QPropertyAnimation * a7 = new QPropertyAnimation(q, "pos", q);
        a7->setDuration(100);
        a7->setStartValue(nowPos);
        a7->setEndValue(nowPos = QPointF(nowPos.x() - 100, nowPos.y()));


        QSequentialAnimationGroup* group = new QSequentialAnimationGroup(q);
        group->addAnimation(a1);
        group->addAnimation(a2);
        group->addAnimation(a3);
        group->addAnimation(a4);
        group->addAnimation(a5);
        group->addAnimation(a6);
        group->addAnimation(a7);

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
    if(!d->pAnimationGroup)
    {
        d->initAnimation();
    }

    if(d->pAnimationGroup->state() != QAnimationGroup::Stopped)
    {
        return;
    }
    d->pAnimationGroup->start();
}

ShakeLabel::ShakeLabel(ShakeLabelPrivate& d, QWidget* parent, Qt::WindowFlags f)
    :QLabel(parent, f)
    ,d_ptr(&d)
{

}

#include "nightdaytogglebutton.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimeLine>

namespace {
    const int TIME_DURATION = 500;
    const int UPDATE_TIME_DURATION = 5;
    const int TIME_DURATION_STEP = TIME_DURATION / UPDATE_TIME_DURATION;
}

class NightDayToggleButtonPrivate{
    enum UpdateState{
        None,
        toDayIng,
        toNightIng
    };

    NightDayToggleButtonPrivate(NightDayToggleButton* q)
        :_q(q)
    {
    }

    QRectF getRoundRectF();
    QPainterPath getRoundPainterPath();
    QRectF getLeftSquareRectF();
    QRectF getRightSquareRectF();
    int getMargin();
    QRectF getSunRectF();
    QList<QPainterPath> getSunHaloRectFs();
    QPainterPath getBackCloudPainterPath();

    void toDayOrToNight();
    void stopToDayToToNight();

    bool mIsPressing = false;
    NightDayToggleButton::State mState = NightDayToggleButton::Day;
    UpdateState mUpdateState = None;
    int mSunPos = -1;
    int mSunStart = -1;
    int mSunEnd = -1;
    int mSunMoveStep = 1;

    int mUpdateTimerId = -1;

    bool mHasDoBack = false;

    double mBackOpacity = 0.00;
    double mBackOpacityStep = 0.1;

    int mCloudDown = 0;
    int mCloudDownStep = 1;

    NightDayToggleButton* _q;
    friend class NightDayToggleButton;
};

QRectF NightDayToggleButtonPrivate::getRoundRectF()
{
    return _q->rect().marginsRemoved(QMargins(2,2,2,2));
}

QPainterPath NightDayToggleButtonPrivate::getRoundPainterPath()
{
    QRectF roundRectF = getRoundRectF();
    QPainterPath roundRectPath;
    roundRectPath.addRoundedRect(roundRectF, roundRectF.height() / 2, roundRectF.height() / 2);
    return std::move(roundRectPath);
}

QRectF NightDayToggleButtonPrivate::getLeftSquareRectF()
{
    QRectF roundRectF = getRoundRectF();
    int h = roundRectF.height();
    return std::move(QRectF(roundRectF.left(), roundRectF.top(), h, h));
}

QRectF NightDayToggleButtonPrivate::getRightSquareRectF()
{
    QRectF roundRectF = getRoundRectF();
    int h = roundRectF.height();
    return std::move(QRectF(roundRectF.right() - h, roundRectF.top(), h,h));
}

int NightDayToggleButtonPrivate::getMargin()
{
    QRectF leftRectF = getLeftSquareRectF();
    QPointF leftCenter = leftRectF.center();
    int margin = qMin<int>(20, leftRectF.height() / 6);
    return margin;
}

QRectF NightDayToggleButtonPrivate::getSunRectF()
{
    if(mState == NightDayToggleButton::Day)
    {
        QRectF leftRectF = getLeftSquareRectF();
        QPointF leftCenter = leftRectF.center();
        int margin = getMargin();
        QRectF sunRectF = leftRectF.marginsRemoved(QMargins(margin,margin,margin,margin));
        if(mSunPos != -1)
        {
            sunRectF.moveCenter(QPointF(mSunPos, leftCenter.y()));
        }
        return std::move(sunRectF);
    }
    else
    {
        QRectF rightRectF = getRightSquareRectF();
        QPointF rightCenter = rightRectF.center();
        int margin = qMin<int>(20, rightRectF.height() / 6);
        QRectF sunRectF = rightRectF.marginsRemoved(QMargins(margin,margin,margin,margin));
        if(mSunPos != -1)
        {
            sunRectF.moveCenter(QPointF(mSunPos, rightCenter.y()));
        }
        return std::move(sunRectF);
    }
}

QList<QPainterPath> NightDayToggleButtonPrivate::getSunHaloRectFs()
{
    QRectF sunRectF = getSunRectF();

    QList<QPainterPath> list;
    QPainterPath shadowPath;
    shadowPath.addEllipse(sunRectF.center(), sunRectF.height(), sunRectF.height());
    QPainterPath shadowPath1;
    shadowPath1.addEllipse(sunRectF.center(), sunRectF.height() / 2 * 3, sunRectF.height() / 2 * 3);
    QPainterPath shadowPath2;
    shadowPath2.addEllipse(sunRectF.center(), sunRectF.height() / 2 * 4, sunRectF.height() / 2 * 4);
    list << shadowPath << shadowPath1 << shadowPath2;

    return std::move(list);
}

QPainterPath NightDayToggleButtonPrivate::getBackCloudPainterPath()
{
    QRectF roundRectF = getRoundRectF();
    int cloudWidth = roundRectF.width() / 3;

    QPainterPath cloud;
    cloud.addEllipse(roundRectF.left(), roundRectF.bottom() - roundRectF.height() / 6 + mCloudDown, cloudWidth, cloudWidth);
    QRectF cloudRectF = cloud.boundingRect();
    QPainterPath cloud1;
    cloud1.addEllipse(cloudRectF.center().x(), cloudRectF.top() - cloudWidth / 5, cloudWidth, cloudWidth);
    QRectF cloudRectF1 = cloud1.boundingRect();
    QPainterPath cloud2;
    cloud2.addEllipse(cloudRectF1.center().x(), cloudRectF1.top() - cloudWidth / 10, cloudWidth, cloudWidth);
    QRectF cloudRectF2 = cloud2.boundingRect();
    QPainterPath cloud3;
    cloud3.addEllipse(cloudRectF2.center().x(), cloudRectF2.top() + cloudWidth / 8, cloudWidth, cloudWidth);
    QRectF cloudRectF3 = cloud3.boundingRect();
    QPainterPath cloud4;
    cloud4.addEllipse(cloudRectF3.center().x() - cloudWidth / 8, cloudRectF3.top() - cloudWidth / 4, cloudWidth, cloudWidth);
    QRectF cloudRectF4 = cloud4.boundingRect();
    QPainterPath cloud5;
    cloud5.addEllipse(cloudRectF4.center().x() - cloudWidth / 8, cloudRectF4.top() - cloudWidth / 8, cloudWidth, cloudWidth);
    QRectF cloudRectF5 = cloud5.boundingRect();
    QPainterPath cloud6;
    cloud6.addEllipse(cloudRectF5.center().x() - cloudWidth / 8, cloudRectF5.top() - cloudWidth / 3, cloudWidth, cloudWidth);

    QPainterPath allCloud;
    allCloud += cloud;
    allCloud += cloud1;
    allCloud += cloud2;
    allCloud += cloud3;
    allCloud += cloud4;
    allCloud += cloud5;
    allCloud += cloud6;

    return std::move(allCloud);
}

void NightDayToggleButtonPrivate::toDayOrToNight()
{
    if(mUpdateState != None)
    {
        return;
    }
    if(mState == NightDayToggleButton::Day)
    {
        mUpdateState = toNightIng;

        int start = getLeftSquareRectF().center().x();
        int end = getRightSquareRectF().center().x();

        mSunStart = start;
        mSunEnd = end;
        mSunPos = start;
        mSunMoveStep = qMax((end - start) / (TIME_DURATION_STEP), 1);

        mBackOpacity = 0.00;
        mBackOpacityStep = 1.0 / (TIME_DURATION_STEP);

        mCloudDown = 0;
        mCloudDownStep = 1;
        // mCloudDownStep = getLeftSquareRectF().height() * 2 / TIME_DURATION_STEP;

        qDebug() << "toDayOrToNight:" << "mSunMoveStep" << mSunMoveStep << start << end << mBackOpacityStep;
        mUpdateTimerId = _q->startTimer(UPDATE_TIME_DURATION, Qt::CoarseTimer);
    }
    else
    {
        mUpdateState = toDayIng;

        int start = getLeftSquareRectF().center().x();
        int end = getRightSquareRectF().center().x();
        mSunStart = start;
        mSunEnd = end;
        mSunPos = end;
        mSunMoveStep = qMax((end - start) / (TIME_DURATION_STEP), 1);

        mBackOpacity = 1.00;
        mBackOpacityStep = 1.0 / (TIME_DURATION_STEP);

        mCloudDown = TIME_DURATION_STEP;
        mCloudDownStep = 1;
        // mCloudDownStep = (getLeftSquareRectF().height() * 1.5) / TIME_DURATION_STEP;

        qDebug() << "toDayOrToNight:" << "mSunMoveStep" << mSunMoveStep << start << end << mBackOpacityStep;
        mUpdateTimerId = _q->startTimer(UPDATE_TIME_DURATION, Qt::CoarseTimer);
    }
}

void NightDayToggleButtonPrivate::stopToDayToToNight()
{
    _q->killTimer(mUpdateTimerId);
    mUpdateTimerId = -1;

    mState = mState == NightDayToggleButton::Day ? NightDayToggleButton::Night : NightDayToggleButton::Day;
    mUpdateState = NightDayToggleButtonPrivate::None;

    mSunPos = -1;
    mSunStart = -1;
    mSunEnd = -1;
    mSunMoveStep = 1;

    mHasDoBack = false;

    mBackOpacity = mState == NightDayToggleButton::Day ? 0.0 : 1.0;
    qDebug() << mBackOpacity;
    mBackOpacityStep = 0.1;
}

/**************************************************/

NightDayToggleButton::NightDayToggleButton(QWidget *parent)
    : QWidget{parent}
    , _d(new NightDayToggleButtonPrivate(this))
{
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

NightDayToggleButton::State NightDayToggleButton::state() const
{
    return _d->mState;
}

void NightDayToggleButton::setState(State st)
{
    if(_d->mState != st)
    {
        // _d->mState = st;
        _d->toDayOrToNight();
    }
}

void NightDayToggleButton::mousePressEvent(QMouseEvent* event)
{
    if(_d->getRoundPainterPath().contains(event->pos()))
    {
        _d->mIsPressing = true;
    }
}

void NightDayToggleButton::mouseReleaseEvent(QMouseEvent* event)
{
    if(_d->mIsPressing)
    {
        _d->toDayOrToNight();
        _d->mIsPressing = false;
    }
}

void NightDayToggleButton::timerEvent(QTimerEvent* event)
{
    if(event->timerId() == _d->mUpdateTimerId)
    {
        if(_d->mUpdateState == NightDayToggleButtonPrivate::toNightIng)
        {
            if(!_d->mHasDoBack)
            {
                _d->mSunPos -= _d->mSunMoveStep;
            }
            else
            {
                _d->mSunPos += _d->mSunMoveStep;
            }

            _d->mBackOpacity += _d->mBackOpacityStep;

            _d->mCloudDown += _d->mCloudDownStep;

            update();

            int margin = _d->getMargin();
            if(_d->mSunPos <= (_d->mSunStart - margin))
            {
                _d->mHasDoBack = true;
            }

            if(_d->mSunPos >= _d->mSunEnd)
            {
                _d->stopToDayToToNight();
            }
        }
        else if(_d->mUpdateState == NightDayToggleButtonPrivate::toDayIng)
        {
            if(!_d->mHasDoBack)
            {
                _d->mSunPos += _d->mSunMoveStep;
            }
            else
            {
                _d->mSunPos -= _d->mSunMoveStep;
            }

            _d->mBackOpacity -= _d->mBackOpacityStep;

            _d->mCloudDown -= _d->mCloudDownStep;

            update();

            int margin = _d->getMargin();
            if(_d->mSunPos >= (_d->mSunEnd + margin))
            {
                _d->mHasDoBack = true;
            }

            if(_d->mSunPos <= _d->mSunStart)
            {
                _d->stopToDayToToNight();
            }
        }
    }
}

void NightDayToggleButton::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    // p.setBrush(Qt::red);
    p.setPen(Qt::transparent);

    QRectF roundRectF = _d->getRoundRectF();
    QPainterPath roundPainterPath = _d->getRoundPainterPath();

    QPainterPath sunPainterPath;
    sunPainterPath.addEllipse(_d->getSunRectF());

    int cloudWidth = roundRectF.width() / 3;
    // QPainterPath ccloud;
    // ccloud.addEllipse(roundRectF.left(),roundRectF.bottom() - 5, roundRectF.height(), roundRectF.height());
    // QPainterPath ccloud1;
    // ccloud1.addEllipse(ccloud.boundingRect().left() + 70, ccloud.boundingRect().top() - 15, ccloud.boundingRect().width(), ccloud.boundingRect().width());
    // QPainterPath ccloud2;
    // ccloud2.addEllipse(ccloud1.boundingRect().left() + 70, ccloud1.boundingRect().top() - 10, ccloud1.boundingRect().width(), ccloud1.boundingRect().width());
    // QPainterPath ccloud3;
    // ccloud3.addEllipse(ccloud2.boundingRect().left() + 80, ccloud2.boundingRect().top() + 10, ccloud2.boundingRect().width(), ccloud2.boundingRect().width());
    // QPainterPath ccloud4;
    // ccloud4.addEllipse(ccloud3.boundingRect().left() + 70, ccloud3.boundingRect().top() - 15, ccloud3.boundingRect().width(), ccloud3.boundingRect().width());
    // QPainterPath ccloud5;
    // ccloud5.addEllipse(ccloud4.boundingRect().left() + 100, ccloud4.boundingRect().top() - 60, ccloud4.boundingRect().width(), ccloud4.boundingRect().width());
    // QPainterPath ccloud6;
    // ccloud6.addEllipse(ccloud5.boundingRect().left() + 50, ccloud5.boundingRect().top() - 60, ccloud5.boundingRect().width(), ccloud5.boundingRect().width());

    // QPainterPath callCloud;
    // callCloud += ccloud;
    // callCloud += ccloud1;
    // callCloud += ccloud2;
    // callCloud += ccloud3;
    // callCloud += ccloud4;
    // callCloud += ccloud5;
    // callCloud += ccloud6;



    // 背景
    p.save();
    p.setOpacity(1 - _d->mBackOpacity);
    p.setBrush(QColor("#4378AC"));
    p.drawPath(roundPainterPath);
    p.setOpacity(_d->mBackOpacity);
    p.setBrush(QColor("#1D1F2C"));
    p.drawPath(roundPainterPath);
    p.restore();

    // 远景云
    p.save();
    p.setOpacity(0.6);
    p.setBrush(Qt::white);
    p.drawPath(roundPainterPath - (roundPainterPath - _d->getBackCloudPainterPath()));
    p.restore();

    // // 近景云
    // p.save();
    // p.setBrush(Qt::white);
    // p.drawPath(roundPainterPath - (roundPainterPath - callCloud));
    // p.restore();

    // 太阳光晕
    p.save();
    p.setOpacity(0.3);
    p.setBrush(Qt::white);
    for(auto path : _d->getSunHaloRectFs())
    {
        p.drawPath(roundPainterPath - (roundPainterPath - path));
    }
    p.restore();

    // 太阳
    p.save();
    p.setBrush(QColor("#ecc53f"));
    p.drawPath(sunPainterPath);
    p.restore();

    // p.save();
    // p.setPen(Qt::blue);
    // p.drawLine(_d->getLeftSquareRectF().center(), _d->getRightSquareRectF().center());
    // p.restore();
}


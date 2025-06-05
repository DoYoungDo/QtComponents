#include "loadingwidget.h"

#include <QMap>
#include <QPainter>
#include <QTimerEvent>
#include <QWidget>

namespace {
const int TIME_INTEVEL = 5; // MS
}

struct LoadingWidgetStateInfo{
    int timerId = -1;
    bool disabled = false;
    bool expanding = true;
    int radius = 5;
};

class StyleOptionLoading : public QStyleOption
{
public:
    inline int nextRotate(){return rotate > 360 ? rotate = 0 : rotate+=2;}
    inline int curRotate(){return rotate;}
    inline int radius(){return isExpanding ? qMin(this->rect.width(), this->rect.height()) / 2 : r;}
    void setExpanding(bool is){isExpanding = is;}
    void setRadius(int radius){r = radius;}
private:
    int r = 5;
    int rotate = 0;
    bool isExpanding = false;
};


class LoadingWidgetPrivate{
    LoadingWidgetPrivate(LoadingWidget* q):_q(q){}
    ~LoadingWidgetPrivate()
    {
        qDeleteAll(mLoadingOption);
    }

    QWidget* getWidgetFromTimerId(int timerId)
    {
        for(QWidget* w : mLoadingHandle.keys())
        {
            if(mLoadingHandle.value(w).timerId == timerId)
            {
                return w;
            }
        }
        return nullptr;
    }

    void paintWidget(QWidget*w)
    {
        QStyleOption* option = mLoadingOption.value(w, nullptr);
        if(!option)
        {
            option = _q->createOption();
            mLoadingOption.insert(w, option);
        }

        _q->initStyleOption(option, w);

        QPainter p(w);
        _q->drawLoading(&p, option);
    }

    void markTimerWidget(QWidget* w)
    {
        mLoadingMark.insert(w, true);
    }
    void cancelMarkTimerWidget(QWidget* w)
    {
        mLoadingMark.remove(w);
    }
    bool TimerWidgetMarked(QWidget* w)
    {
        return mLoadingMark.value(w, false);
    }
private:
    LoadingWidget* _q;
    friend class LoadingWidget;
    QMap<QWidget*, LoadingWidgetStateInfo> mLoadingHandle;
    QMap<QWidget*, QStyleOption*> mLoadingOption;
    QMap<QWidget*, bool> mLoadingMark;
};

LoadingWidget::LoadingWidget(QObject *parent)
    : QObject{parent}
    , _d(new LoadingWidgetPrivate(this))
{

}

LoadingWidget::~LoadingWidget()
{
    if(_d)
    {
        delete _d;
        _d = nullptr;
    }
}

void LoadingWidget::startLoading(QWidget* w)
{
    startLoading(w, false);
}

void LoadingWidget::startLoading(QWidget* w, bool disable)
{
    startLoading(w, disable, true);
}

void LoadingWidget::startLoading(QWidget* w, bool disable, bool expanding)
{
    if(_d->mLoadingHandle.contains(w))
    {
        return;
    }

    w->installEventFilter(this);
    w->setDisabled(disable);
    _d->mLoadingHandle.insert(w, LoadingWidgetStateInfo{
                                  this->startTimer(TIME_INTEVEL, Qt::PreciseTimer),
                                  disable,
                                  expanding
                              });
}

void LoadingWidget::startLoading(QWidget* w, bool disable, int radius)
{
    if(_d->mLoadingHandle.contains(w))
    {
        return;
    }

    w->installEventFilter(this);
    w->setDisabled(disable);
    _d->mLoadingHandle.insert(w, LoadingWidgetStateInfo{
                                  this->startTimer(TIME_INTEVEL, Qt::PreciseTimer),
                                  disable,
                                  false,
                                  radius
                              });
}

void LoadingWidget::stopLoading(QWidget* w)
{
    if(!_d->mLoadingHandle.contains(w))
    {
        return;
    }

    LoadingWidgetStateInfo info = _d->mLoadingHandle.take(w);
    w->removeEventFilter(this);
    if(info.disabled)
    {
        w->setDisabled(false);
    }
    this->killTimer(info.timerId);
    w->repaint();

    QStyleOption* option = _d->mLoadingOption.take(w);
    delete option;
}

QStyleOption* LoadingWidget::createOption()
{
    return new StyleOptionLoading;
}

void LoadingWidget::initStyleOption(QStyleOption* opt, QWidget* w)
{
    opt->initFrom(w);

    StyleOptionLoading* option = static_cast<StyleOptionLoading*>(opt);
    if(option)
    {
        LoadingWidgetStateInfo info = _d->mLoadingHandle.value(w);
        option->setRadius(info.radius);
        option->setExpanding(info.expanding);
    }
}

void LoadingWidget::drawLoading(QPainter* p, QStyleOption* opt)
{
    StyleOptionLoading* option = static_cast<StyleOptionLoading*>(opt);

    p->translate(opt->rect.center());

    QTransform tran(p->transform());
    tran.rotate(option->nextRotate());
    p->setTransform(tran);

    const int penWidth = 2;
    int r = option->radius() - penWidth;
    int wAH = r * 2;

    QPen pen;
    pen.setStyle(Qt::DotLine);
    pen.setWidth(penWidth);
    pen.setColor(Qt::gray);
    p->setPen(pen);

    p->drawEllipse(-r, -r, wAH, wAH);
}

bool LoadingWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::Paint)
    {
        // qDebug() << "widget paint event";
        QWidget* w = qobject_cast<QWidget*>(watched);
        // w->QObject::event(event);
        if(_d->TimerWidgetMarked(w))
        {
            _d->paintWidget(w);
            _d->cancelMarkTimerWidget(w);
        }
        return false;
    }
    return QObject::eventFilter(watched, event);
}

void LoadingWidget::timerEvent(QTimerEvent* event)
{
    QWidget* w = _d->getWidgetFromTimerId(event->timerId());
    Q_ASSERT(w);

    _d->markTimerWidget(w);
    w->repaint();
}



PictureLoadingWidget::PictureLoadingWidget(QObject* parent)
    :LoadingWidget(parent)
{

}

void PictureLoadingWidget::drawLoading(QPainter* p, QStyleOption* opt)
{
    StyleOptionLoading* option = static_cast<StyleOptionLoading*>(opt);

    p->translate(opt->rect.center());

    QTransform tran(p->transform());
    tran.rotate(option->nextRotate());
    p->setTransform(tran);

    int r = option->radius();
    // qDebug() << "r:" << r << option->rect;
    int wAH = r * 2;

    p->drawPixmap(-r, -r, wAH, wAH, QPixmap(":/loading.png"));
}

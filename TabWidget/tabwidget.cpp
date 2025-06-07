#include "tabwidget.h"

#include <QBoxLayout>
#include <QDrag>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSplitter>
#include <QStyleOptionTabWidgetFrame>

namespace {
const char* TYPE_PAGE_WIDGET = "type.page.widget";
const char* TYPE_FROM_TAB_WIDGET = "type.from.tab.widget";
const char* TYPE_CONTAINER_WIDGET = "type.container.widget";
const char* TYPE_TITLE = "type.title";

const qreal MASK_OPACITY = 0.3;
// const char* TYPE_ORIENTATION = "type.orientation";
}

class TabMoveMimeDataPrivate{
    TabMoveMimeDataPrivate(TabMoveMimeData* q):_q(q){}

private:
    QMap<QString, QVariant> mDatas;

    TabMoveMimeData* _q;
    friend class TabMoveMimeData;
};
/************************* ⬆︎ TabMoveMimeDataPrivate ⬆︎ *************************/

TabMoveMimeData::TabMoveMimeData()
    :_d(new TabMoveMimeDataPrivate(this))
{

}

void TabMoveMimeData::setPage(QWidget* page)
{
    _d->mDatas.insert(TYPE_PAGE_WIDGET, QVariant::fromValue<QWidget*>(page));
}

QWidget* TabMoveMimeData::page() const
{
    return _d->mDatas.value(TYPE_PAGE_WIDGET).value<QWidget*>();
}

void TabMoveMimeData::setTitle(const QString& title)
{
    _d->mDatas.insert(TYPE_TITLE, title);
}

QString TabMoveMimeData::title() const
{
    return _d->mDatas.value(TYPE_TITLE).toString();
}

void TabMoveMimeData::setFromTabWidget(TabWidget* from)
{
    _d->mDatas.insert(TYPE_FROM_TAB_WIDGET, QVariant::fromValue<TabWidget*>(from));
}

TabWidget* TabMoveMimeData::fromTabWidget() const
{
    return _d->mDatas.value(TYPE_FROM_TAB_WIDGET).value<TabWidget*>();
}

void TabMoveMimeData::setContainer(TabContainer* container)
{
    _d->mDatas.insert(TYPE_CONTAINER_WIDGET, QVariant::fromValue<TabContainer*>(container));
}

TabContainer* TabMoveMimeData::container() const
{
    return _d->mDatas.value(TYPE_CONTAINER_WIDGET).value<TabContainer*>();
}

/************************* ⬆︎ TabMoveMimeData ⬆︎ *************************/

class TabBarPrivate{
    TabBarPrivate(TabBar* q):_q(q){}

private:
    bool isMousePressed = false;
    QPoint pressedPos;

    TabWidget* pTabWidget = nullptr;

    TabBar* _q;
    friend class TabBar;
};
/************************* ⬆︎ TabBarPrivate ⬆︎ *************************/

class TabWidgetPrivate{
    TabWidgetPrivate(TabWidget* q):_q(q)
    {
        pTabbar = new TabBar(_q);
        _q->connect(pTabbar, &TabBar::tabDraged, _q, &TabWidget::onTabDraged);

        _q->setTabBar(pTabbar);
    }

    bool canTrop(const TabMoveMimeData *data,QPointF pos)
    {
        TabWidget* from = data->fromTabWidget();
        if(from == _q && (mShowMaskOrientation == TabWidget::CENTER || from->count() == 1)){
            return false;
        }

        QWidget* curWidget = _q->currentWidget();
        QRect curRect = curWidget->geometry();
        QPoint curWidgetPos = curWidget->mapFromGlobal(_q->mapToGlobal(pos.toPoint()));

        return curRect.contains(curWidgetPos);
    }
    void showTabMask(QPointF pos)
    {
        mShowMask = true;

        QWidget* curWidget = _q->currentWidget();
        QRect curRect = curWidget->geometry();

        int halfWidth = curRect.width() / 2;
        int halfHeight = curRect.height() / 2;
        int twentyPresentsHalfWidth = halfWidth / 5;
        int twentyPresentshalfHeight = halfHeight / 5;

        QPoint curWidgetPos = curWidget->mapFromGlobal(_q->mapToGlobal(pos.toPoint()));

        QRect leftRect = curRect.marginsRemoved(QMargins(0,0,twentyPresentsHalfWidth * 9,0));
        QRect topRect = curRect.marginsRemoved(QMargins(twentyPresentsHalfWidth,0,twentyPresentsHalfWidth,twentyPresentshalfHeight * 9));
        QRect rightRect = curRect.marginsRemoved(QMargins(twentyPresentsHalfWidth*9,0,0,0));
        QRect bottomRect = curRect.marginsRemoved(QMargins(twentyPresentsHalfWidth,twentyPresentshalfHeight*9,twentyPresentsHalfWidth,0));
        QRect centerRect = QRect(topRect.left(), topRect.bottom(), topRect.width(), bottomRect.top() - topRect.bottom());
        // qDebug() << "event pos" << pos << "rects" << curRect << leftRect << topRect << rightRect << bottomRect;


        if(leftRect.contains(curWidgetPos))
        {
            mShowMaskRect = curRect.marginsRemoved(QMargins(0,0,halfWidth,0));
            // mShowMaskRect = leftRect;
            mShowMaskOrientation = TabWidget::LEFT;
        }
        else if(topRect.contains(curWidgetPos))
        {
            mShowMaskRect = curRect.marginsRemoved(QMargins(0,0,0,halfHeight));
            // mShowMaskRect = topRect;
            mShowMaskOrientation = TabWidget::TOP;
        }
        else if(rightRect.contains(curWidgetPos))
        {
            mShowMaskRect = curRect.marginsRemoved(QMargins(halfWidth,0,0,0));
            // mShowMaskRect = rightRect;
            mShowMaskOrientation = TabWidget::RIGHT;
        }
        else if(bottomRect.contains(curWidgetPos))
        {
            mShowMaskRect = curRect.marginsRemoved(QMargins(0,halfHeight,0,0));
            // mShowMaskRect = bottomRect;
            mShowMaskOrientation = TabWidget::BOTTOM;
        }
        else if(centerRect.contains(curWidgetPos))
        {
            mShowMaskRect = curRect;
            mShowMaskOrientation = TabWidget::CENTER;
        }
        else
        {
            mShowMask = false;
        }

        _q->update();
    }
    void hideTabMask()
    {
        if(!mShowMask)
        {
            return;
        }

        mShowMask = false;
        _q->update();
    }
    QColor invertColor(const QColor &color) {
        return QColor(
            255 - color.red(),
            255 - color.green(),
            255 - color.blue(),
            color.alpha()  // 保留透明度
            );
    }
    QPixmap createPixmap(const QString& title)
    {
        QFontMetrics metriceF = _q->fontMetrics();

        int w = metriceF.horizontalAdvance(title) + 20;
        int h = 30;
        QPixmap pixmap(w, h);
        pixmap.fill(Qt::transparent);

        QPainter p(&pixmap);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(pTabbar->palette().brush(QPalette::Button).color());
        p.drawRoundedRect(pixmap.rect(), 5, 5);

        p.setPen(pTabbar->palette().color(QPalette::ButtonText));
        p.drawText(pixmap.rect(), Qt::AlignCenter, title);

        return std::move(pixmap);
    }
private:
    TabBar* pTabbar = nullptr;
    bool mShowMask = false;
    QRect mShowMaskRect;
    TabWidget::Orientations mShowMaskOrientation = TabWidget::CENTER;
    TabContainer* pContainer = nullptr;
    TabWidget* _q;
    friend class TabWidget;
    friend class TabContainerPrivate;
};
/************************* ⬆︎ TabWidgetPrivate ⬆︎ *************************/

class TabSplitterPrivate
{
    TabSplitterPrivate(TabSplitter* q):_q(q){}

private:
    TabSplitter* _q;
    friend class TabSplitter;

    QList<QWidget*> mValidWidgets;
};
/************************* ⬆︎ TabSplitterPrivate ⬆︎ *************************/

class TabContainerPrivate{
private:
    TabContainerPrivate(TabContainer* q):_q(q)
    {
        setupUi();
    }

    void setupUi()
    {
        pMainLayoutSplitter = new TabSplitter(_q);

        QBoxLayout* layout = new QBoxLayout(QBoxLayout::Down, _q);
        layout->setSpacing(0);
#ifdef TAB_TEST
        layout->setContentsMargins(10,10,10,10);
#else
        layout->setContentsMargins(0,0,0,0);
#endif

        layout->addWidget(pMainLayoutSplitter);
    }

    TabWidget* createTabWidget()
    {
        TabWidget* tabWidget = new TabWidget(_q);
        return tabWidget;
    }

    TabContainer* createContainer()
    {
        TabContainer* container = new TabContainer();
        container->_d->pParentContainer = _q;

        return  container;
    }

    inline bool isContainer(QWidget* w)
    {
        return w->inherits("TabContainer");
    }

    bool isInvalidWidget(QWidget* w)
    {
        return w->inherits("InvalidWidget");
    }

    bool isAnyOrirentaion()
    {
        return pMainLayoutSplitter->widgetCount() == 1;
    }

    void rebuildStructure()
    {
        if(pParentContainer)
        {
            int count = pParentContainer->_d->pMainLayoutSplitter->widgetCount();

            QList<TabContainer*> containers;
            QList<TabWidget*> tabs;
            for(int i = 0;i < count;++i)
            {
                QWidget* w = pParentContainer->_d->pMainLayoutSplitter->widget(i);
                Q_ASSERT(w);
                if(isContainer(w))
                {
                    containers << qobject_cast<TabContainer*>(w);
                }
                else
                {
                    tabs << qobject_cast<TabWidget*>(w);
                }
            }

            for(TabContainer* container : containers)
            {
                if(container->_d->pMainLayoutSplitter->isEmpty())
                {
                    pParentContainer->_d->pMainLayoutSplitter->removeWidget(container);
                    continue;
                }
                else
                {
                    if(pParentContainer->_d->isAnyOrirentaion())
                    {
                        pParentContainer->_d->pMainLayoutSplitter->setOrientation(container->_d->pMainLayoutSplitter->orientation());

                        QList<QWidget*> widgets = container->_d->pMainLayoutSplitter->takeAll();
                        pParentContainer->_d->pMainLayoutSplitter->removeWidget(container);

                        for(QWidget* widget: widgets)
                        {
                            if(isContainer(widget))
                            {
                                qobject_cast<TabContainer*>(widget)->_d->pParentContainer = pParentContainer;
                            }
                            else
                            {
                                qobject_cast<TabWidget*>(widget)->_d->pContainer = pParentContainer;
                            }

                            *pParentContainer->_d->pMainLayoutSplitter << widget;
                        }
                        continue;
                    }
                    else
                    {
                        if(container->_d->isAnyOrirentaion())
                        {
                            QWidget* w = container->_d->pMainLayoutSplitter->tabkeWidget(0);

                            if(isContainer(w))
                            {
                                qobject_cast<TabContainer*>(w)->_d->pParentContainer = pParentContainer;
                            }
                            else
                            {
                                qobject_cast<TabWidget*>(w)->_d->pContainer = pParentContainer;
                            }
                            pParentContainer->_d->pMainLayoutSplitter->replaceWidget(pParentContainer->_d->pMainLayoutSplitter->indexOf(container), w);
                            container->deleteLater();
                            continue;
                        }
                        else if(container->_d->pMainLayoutSplitter->orientation() == pParentContainer->_d->pMainLayoutSplitter->orientation())
                        {

                        }
                    }
                }
            }

            pParentContainer->_d->rebuildStructure();
        }
    }
private:
    TabSplitter* pMainLayoutSplitter = nullptr;

    TabContainer* _q = nullptr;
    friend class TabContainer;
    friend class TabWidget;
    TabContainer* pParentContainer = nullptr;
};
/************************* ⬆︎ TabContainerPrivate ⬆︎ *************************/

TabBar::TabBar(QWidget* parent)
    :QTabBar(parent)
    ,_d(new TabBarPrivate(this))
{
    this->setAcceptDrops(true);
}

void TabBar::mousePressEvent(QMouseEvent* event)
{
    _d->isMousePressed = true;
    _d->pressedPos = event->pos();
    QTabBar::mousePressEvent(event);
}

void TabBar::mouseReleaseEvent(QMouseEvent* event)
{
    _d->isMousePressed = true;
    QTabBar::mouseReleaseEvent(event);
}

void TabBar::mouseMoveEvent(QMouseEvent* event)
{
    if(_d->isMousePressed)
    {
        emit tabDraged(tabAt(_d->pressedPos));
        return;
    }

    QTabBar::mouseMoveEvent(event);
}
/************************* ⬆︎ TabBar ⬆︎ *************************/

TabWidget::TabWidget(TabContainer* container, QWidget* parent)
    :QTabWidget(parent)
    ,_d(new TabWidgetPrivate(this))
{
    this->setAcceptDrops(true);
    this->setUsesScrollButtons(true);

    _d->pContainer = container;
}

void TabWidget::dragEnterEvent(QDragEnterEvent* event)
{
    const TabMoveMimeData *data = qobject_cast<const TabMoveMimeData *>(event->mimeData());
    // qDebug() << "enter data" << (void*)data;
    if (data)
    {
        _d->showTabMask(event->position());
        event->accept();
    }
    QTabWidget::dragEnterEvent(event);
}

void TabWidget::dragMoveEvent(QDragMoveEvent* event)
{
    const TabMoveMimeData *data = qobject_cast<const TabMoveMimeData *>(event->mimeData());
    if (data)
    {
        _d->showTabMask(event->position());
    }
    QTabWidget::dragMoveEvent(event);
}

void TabWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    _d->hideTabMask();

    QTabWidget::dragLeaveEvent(event);
}

void TabWidget::dropEvent(QDropEvent* event)
{
    _d->hideTabMask();

    const TabMoveMimeData *data = qobject_cast<const TabMoveMimeData *>(event->mimeData());
    if(data)
    {
        qDebug() << "get data";

        if(_d->canTrop(data, event->position()))
        {
            QWidget* page = data->page();
            TabWidget* from = data->fromTabWidget();

            int index = from->indexOf(page);
            QString title = from->tabText(index);
            TabContainer* fromContainer = from->_d->pContainer;
            from->removeTab(index);

            if(_d->mShowMaskOrientation == CENTER)
            {
                this->addTab(page, title);
            }
            else
            {
                TabContainer* currentContainer = _d->pContainer;
                TabSplitter* layout = currentContainer->_d->pMainLayoutSplitter;
                int index = layout->indexOf(this);
                Q_ASSERT(index != -1);

                int orientation = (_d->mShowMaskOrientation == TabWidget::LEFT || _d->mShowMaskOrientation == TabWidget::RIGHT) ? Qt::Horizontal : Qt::Vertical;
                if(orientation == layout->orientation())
                {
SAME_ORI:
                    TabWidget* newTab = currentContainer->_d->createTabWidget();
                    newTab->addTab(data->page(), data->title());
                    layout->insertWidget((_d->mShowMaskOrientation == TabWidget::LEFT || _d->mShowMaskOrientation == TabWidget::TOP) ? index : index + 1, newTab);
                }
                else
                {
                    if(currentContainer->_d->isAnyOrirentaion())
                    {
                        layout->setOrientation((Qt::Orientation)orientation);
                        goto SAME_ORI;
                    }

                    TabContainer* newContainer = currentContainer->_d->createContainer();
                    newContainer->_d->pMainLayoutSplitter->setOrientation((Qt::Orientation)orientation);

                    QWidget* oldWidget = currentContainer-> _d->pMainLayoutSplitter->replaceWidget(index, newContainer);
                    Q_ASSERT(this == oldWidget);
                    _d->pContainer = newContainer;

                    TabWidget* newTab = newContainer->_d->createTabWidget();
                    newTab->addTab(page, title);

                    if((_d->mShowMaskOrientation == TabWidget::LEFT || _d->mShowMaskOrientation == TabWidget::TOP))
                    {
                        *newContainer->_d->pMainLayoutSplitter << newTab << oldWidget;
                    }
                    else
                    {
                        *newContainer->_d->pMainLayoutSplitter << oldWidget << newTab;
                    }
                }

            }

            fromContainer->_d->rebuildStructure();
            _d->pContainer->_d->rebuildStructure();

            event->acceptProposedAction();
            return;
        }
    }

    QTabWidget::dropEvent(event);
}

void TabWidget::paintEvent(QPaintEvent* event)
{
    QTabWidget::paintEvent(event);

    if(_d->mShowMask)
    {
        QWidget* curWidget = this->currentWidget();
        QRect curRect = curWidget->geometry();

        QPointF curPos = this->mapFromGlobal(curWidget->mapToGlobal(curRect.topLeft()));

        QPainter p(this);
        p.setOpacity(MASK_OPACITY);
        p.translate(curPos);
        p.setPen(Qt::transparent);
        p.setBrush(_d->invertColor(this->palette().brush(QPalette::Window).color()));
        p.drawRoundedRect(_d->mShowMaskRect, 2, 2);
    }
}

void TabWidget::tabInserted(int index)
{
    // qDebug() << "tab insert" << index << this->count();
    QTabWidget::tabInserted(index);
}

void TabWidget::tabRemoved(int index)
{
    QTabWidget::tabRemoved(index);

    if(this->count() == 0)
    {
        _d->pContainer->_d->pMainLayoutSplitter->removeWidget(this);
    }
}

void TabWidget::onTabDraged(int index)
{
    qDebug() << "tab draged" << index;
    this->setCurrentIndex(index);

    TabMoveMimeData* mimeData = new TabMoveMimeData;
    mimeData->setPage(this->widget(index));
    mimeData->setTitle(this->tabText(index));
    mimeData->setFromTabWidget(this);
    mimeData->setContainer(_d->pContainer);

    QDrag *drag = new QDrag(this);
    drag->setPixmap(_d->createPixmap(mimeData->title()));
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}
/************************* ⬆︎ TabWidget ⬆︎ *************************/

TabSplitter::TabSplitter(QWidget* parent)
    : QSplitter(parent)
    , _d(new TabSplitterPrivate(this))
{
    this->setChildrenCollapsible(false);
}

void TabSplitter::addWidget(QWidget* w)
{
    if(_d->mValidWidgets.contains(w))
    {
        return;
    }

    _d->mValidWidgets << w;
    QSplitter::addWidget(w);
}

void TabSplitter::insertWidget(int index, QWidget* w)
{
    if(_d->mValidWidgets.contains(w))
    {
        return;
    }
    index = index < 0 ? 0 : index;
    if(index < _d->mValidWidgets.size())
    {
        QWidget* old = _d->mValidWidgets.at(index);
        _d->mValidWidgets.insert(index, w);
        QSplitter::insertWidget(QSplitter::indexOf(old), w);
    }
    else
    {
        *this << w;
    }
}

QWidget* TabSplitter::replaceWidget(int index, QWidget* w)
{
    if(index < 0 || index >= _d->mValidWidgets.size())
    {
        return nullptr;
    }

    QWidget* old = _d->mValidWidgets.at(index);
    _d->mValidWidgets.replace(index, w);

    return QSplitter::replaceWidget(QSplitter::indexOf(old), w);
}

int TabSplitter::indexOf(QWidget* w)
{
    return _d->mValidWidgets.indexOf(w);
}

QWidget* TabSplitter::widget(int index)
{
    if(index < 0 || index >= _d->mValidWidgets.size())
    {
        return nullptr;
    }
    return _d->mValidWidgets.at(index);
}

QWidget* TabSplitter::tabkeWidget(int index)
{
    QWidget * w = _d->mValidWidgets.takeAt(index);
    if(w)
    {
        QWidget* placeholder = new QWidget;
        QSplitter::replaceWidget(QSplitter::indexOf(w), placeholder);
        placeholder->deleteLater();
        return w;
    }
    return nullptr;
}

QList<QWidget*> TabSplitter::takeAll()
{
    QList<QWidget*> list = _d->mValidWidgets;
    _d->mValidWidgets.clear();

    for(QWidget* w : list)
    {
        QWidget* placeholder = new QWidget;
        QSplitter::replaceWidget(QSplitter::indexOf(w), placeholder);
        placeholder->deleteLater();
    }

    return list;
}

void TabSplitter::removeWidget(QWidget* w)
{
    if(!_d->mValidWidgets.contains(w))
    {
        return;
    }
    _d->mValidWidgets.removeOne(w);

    w->deleteLater();
}

void TabSplitter::removeWidget(int index)
{
    QWidget* w = _d->mValidWidgets.at(index);
    if(w)
    {
        removeWidget(w);
    }
}

void TabSplitter::removeAll()
{
    int count = QSplitter::count();
    for(int i = 0;i < count; ++i)
    {
        QWidget* w = QSplitter::widget(i);
        w->deleteLater();
    }

    _d->mValidWidgets.clear();
}

int TabSplitter::widgetCount()
{
    return _d->mValidWidgets.size();
}

bool TabSplitter::isEmpty()
{
    return _d->mValidWidgets.isEmpty();
}

QWidget* TabSplitter::first()
{
    return _d->mValidWidgets.first();
}

QWidget* TabSplitter::last()
{
    return _d->mValidWidgets.last();
}

TabSplitter& TabSplitter::operator <<(QWidget* w)
{
    addWidget(w);
    return *this;
}
/************************* ⬆︎ TabSplitter ⬆︎ *************************/

TabContainer::TabContainer(QWidget* parent)
    :QWidget(parent)
    ,_d(new TabContainerPrivate(this))
{

}

void TabContainer::addTab(QWidget* page, const QString& label)
{
    if(_d->pMainLayoutSplitter->isEmpty())
    {
        TabWidget* tab = _d->createTabWidget();
        tab->addTab(page, label);
        _d->pMainLayoutSplitter->addWidget(tab);
    }
    else
    {
        QWidget* ele = _d->pMainLayoutSplitter->last();
        if(_d->isContainer(ele))
        {
            qobject_cast<TabContainer*>(ele)->addTab(page, label);
        }
        else
        {
            qobject_cast<TabWidget*>(ele)->addTab(page, label);
        }
    }
}

void TabContainer::addTab(QWidget* page, const QString& label, bool split)
{
    if(!split)
    {
        addTab(page, label);
        return;
    }

    TabWidget* tab = _d->createTabWidget();
    tab->addTab(page, label);
    _d->pMainLayoutSplitter->addWidget(tab);
}

void TabContainer::removeAll()
{
    _d->pMainLayoutSplitter->removeAll();
}

#ifdef TAB_TEST

void TabContainer::print()
{
    qDebug() << "count" << _d->pMainLayoutSplitter->count() << "valid count" << _d->pMainLayoutSplitter->widgetCount();
}

void TabContainer::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setPen(_d->pMainLayoutSplitter->orientation() == Qt::Horizontal ? Qt::red : Qt::green);
    p.setBrush(Qt::transparent);
    p.drawRect(this->rect().marginsRemoved(QMargins(1,1,1,1)));
}
#endif
/************************* ⬆︎ TabContainer ⬆︎ *************************/

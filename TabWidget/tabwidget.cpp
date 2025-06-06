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

// void TabMoveMimeData::setOrientation(int ori) const
// {
//     _d->mDatas.insert(TYPE_ORIENTATION, ori);
// }

// int TabMoveMimeData::orientation() const
// {
//     return _d->mDatas.value(TYPE_ORIENTATION).toInt();
// }
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
        else
        {
            mShowMaskRect = curRect;
            mShowMaskOrientation = TabWidget::CENTER;
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
    // QRect validRect()
    // {

    // }
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

class TabContainerPrivate{
private:
    TabContainerPrivate(TabContainer* q):_q(q)
    {
        setupUi();
    }

    void setupUi()
    {
        pMainLayoutSplitter = new QSplitter(_q);

        QBoxLayout* layout = new QBoxLayout(QBoxLayout::Down, _q);
        layout->setSpacing(0);
        layout->setContentsMargins(10,10,10,10);

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
        QList<QWidget*> validWidgets;

        int count = pMainLayoutSplitter->count();
        for(int i = 0;i < count;++i)
        {
            QWidget* w = pMainLayoutSplitter->widget(i);
            if(!isInvalidWidget(w))
            {
                validWidgets << w;
            }
        }

        return validWidgets.size() == 1;
    }

    void rebuildStructure()
    {
        QList<QWidget*> validWidgets;
        QList<QWidget*> invalidWidgets;

        int count = pMainLayoutSplitter->count();
        for(int i = 0;i < count;++i)
        {
            QWidget* w = pMainLayoutSplitter->widget(i);
            if(isInvalidWidget(w))
            {
                invalidWidgets << w;
            }
            else
            {
                validWidgets << w;
            }
        }

        qDebug() << "cout" << count << "valid size" <<validWidgets.size() << "invalid size" << invalidWidgets.size();

        int validWidgetCount = validWidgets.size();
        if(validWidgetCount == 1)
        {
            QWidget* w = validWidgets.at(0);
            if(isContainer(w))
            {
                // TabContainer* container = qobject_cast<TabContainer*>(w);

                // pMainLayoutSplitter->setOrientation(container->_d->pMainLayoutSplitter->orientation());

                // int subCount = container->_d->pMainLayoutSplitter->count();
                // for(int i = 0;i < subCount;++i)
                // {
                //     QWidget* subW = container->_d->pMainLayoutSplitter->replaceWidget(i, new InvalidWidget());;
                //     Q_ASSERT(!isInvalidWidget(subW));
                //     if(isContainer(subW))
                //     {
                //         qobject_cast<TabContainer*>(subW)->_d->pParentContainer = _q;
                //     }
                //     else
                //     {
                //         qobject_cast<TabWidget*>(subW)->_d->pContainer = _q;
                //     }

                //     pMainLayoutSplitter->addWidget(subW);
                // }
                // container->deleteLater();
            }
            else
            {
                if(pParentContainer)
                {
                    QWidget* tab = pMainLayoutSplitter->replaceWidget(pMainLayoutSplitter->indexOf(w), new InvalidWidget);
                    Q_ASSERT(tab == w);
                    int index = pParentContainer->_d->pMainLayoutSplitter->indexOf(pMainLayoutSplitter);
                    pParentContainer->_d->pMainLayoutSplitter->replaceWidget(index, tab);
                    pMainLayoutSplitter->deleteLater();
                    return;
                }
            }
        }
        // else
        // {
            // for(int i = 0;i < validWidgetCount;++i)
            // {
            //     QWidget* w = validWidgets.at(i);
            //     if(isContainer(w))
            //     {
            //         TabContainer* container = qobject_cast<TabContainer*>(w);
            //         if(container->_d->pMainLayoutSplitter->orientation() == pMainLayoutSplitter->orientation())
            //         {
            //             int containerIndex = pMainLayoutSplitter->indexOf(container);
            //             int subCount = container->_d->pMainLayoutSplitter->count();

            //             if(subCount > 0)
            //             {
            //                 QList<QWidget*> subWidgets;
            //                 for(int i = 0;i < subCount;++i)
            //                 {
            //                     QWidget* subW = container->_d->pMainLayoutSplitter->replaceWidget(i, new InvalidWidget());;
            //                     Q_ASSERT(!isInvalidWidget(subW));
            //                     subWidgets << subW;
            //                     if(isContainer(subW))
            //                     {
            //                         qobject_cast<TabContainer*>(subW)->_d->pParentContainer = _q;
            //                     }
            //                     else
            //                     {
            //                         qobject_cast<TabWidget*>(subW)->_d->pContainer = _q;
            //                     }
            //                 }

            //                 pMainLayoutSplitter->replaceWidget(containerIndex, subWidgets.takeAt(0));
            //                 int lastCount = subWidgets.size();
            //                 for(int i = 0;i < lastCount;++i)
            //                 {
            //                     pMainLayoutSplitter->insertWidget(containerIndex + i + 1, subWidgets.at(i));
            //                 }
            //             }

            //             container->deleteLater();
            //         }
            //     }
            // }
        // }

        qDeleteAll(invalidWidgets);

        // if(pParentContainer)
        // {
        //     pParentContainer->_d->rebuildStructure();
        // }
    }
private:
    QSplitter* pMainLayoutSplitter = nullptr;

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
    this->installEventFilter(this);

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
            // bool isSameContainer = fromContainer == _d->pContainer;

            from->removeTab(index);

            if(_d->mShowMaskOrientation == CENTER)
            {
                this->addTab(page, title);
                fromContainer->_d->rebuildStructure();
            }
            else
            {
                TabContainer* currentContainer = _d->pContainer;
                QSplitter* layout = currentContainer->_d->pMainLayoutSplitter;
                int index = layout->indexOf(this);
                Q_ASSERT(index != -1);
                // index = index == -1 ? 0 : index;

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
                        newContainer->_d->pMainLayoutSplitter->addWidget(newTab);
                        newContainer->_d->pMainLayoutSplitter->addWidget(oldWidget);
                    }
                    else
                    {
                        newContainer->_d->pMainLayoutSplitter->addWidget(oldWidget);
                        newContainer->_d->pMainLayoutSplitter->addWidget(newTab);
                    }
                }

                if(fromContainer != _d->pContainer)
                {
                    fromContainer->_d->rebuildStructure();
                }
                _d->pContainer->_d->rebuildStructure();
            }

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
        p.setOpacity(0.5);
        p.translate(curPos);
        p.setBrush(Qt::white);
        p.drawRect(_d->mShowMaskRect);
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
        QWidget* old = _d->pContainer->_d->pMainLayoutSplitter->replaceWidget(_d->pContainer->_d->pMainLayoutSplitter->indexOf(this), new InvalidWidget());
        Q_ASSERT(old == this);
        this->deleteLater();
    }
}

void TabWidget::onTabDraged(int index)
{
    qDebug() << "tab draged" << index;
    TabMoveMimeData* mimeData = new TabMoveMimeData;
    mimeData->setPage(this->widget(index));
    mimeData->setTitle(this->tabText(index));
    mimeData->setFromTabWidget(this);
    mimeData->setContainer(_d->pContainer);

    QDrag *drag = new QDrag(this);
    // drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}
/************************* ⬆︎ TabWidget ⬆︎ *************************/

TabContainer::TabContainer(QWidget* parent)
    :QWidget(parent)
    ,_d(new TabContainerPrivate(this))
{

}

void TabContainer::addTab(QWidget* page, const QString& label)
{
    int eleCount = _d->pMainLayoutSplitter->count();
    if(eleCount <= 0)
    {
        TabWidget* tab = _d->createTabWidget();
        tab->addTab(page, label);
        _d->pMainLayoutSplitter->addWidget(tab);
        // _d->mEleWidgetList << tab;
    }
    else
    {
        QWidget* ele = _d->pMainLayoutSplitter->widget(eleCount - 1);
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
    // _d->mEleWidgetList << tab;
}

void TabContainer::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setPen(_d->pMainLayoutSplitter->orientation() == Qt::Horizontal ? Qt::red : Qt::green);
    p.setBrush(Qt::transparent);
    p.drawRect(this->rect().marginsRemoved(QMargins(2,2,2,2)));
}
/************************* ⬆︎ TabContainer ⬆︎ *************************/

void InvalidWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setBrush(Qt::yellow);
    p.drawRect(this->rect().marginsRemoved(QMargins(10,10,10,10)));
}

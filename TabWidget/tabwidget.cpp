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

    TabWidget* createTabWidget(){
        TabWidget* tabWidget = new TabWidget(_q);
        connectTabWidget(tabWidget);
        return tabWidget;
    }
    void connectTabWidget(TabWidget* tabWidget)
    {
        _q->connect(tabWidget, &TabWidget::pageRemoved, _q, &TabContainer::onPageRemoved);
        _q->connect(tabWidget, &TabWidget::pageEntered, _q, &TabContainer::onPageEntered);
    }

    inline bool isContainer(QWidget* w)
    {
        return w->inherits("TabContainer");
    }

    void rebuildStructure()
    {
        if(!pParentContainer)
        {
            return;
        }

        if(mEleWidgetList.size() == 1)
        {
            int index = pParentContainer->_d->pMainLayoutSplitter->indexOf(_q);
            Q_ASSERT(index != -1);

            pParentContainer->_d->pMainLayoutSplitter->replaceWidget(index, mEleWidgetList.at(0));

            pParentContainer->_d->rebuildStructure();

            _q->deleteLater();
        }
        // TabContainer* sdr = qobject_cast<TabContainer*>(this->sender());
        // Q_ASSERT(sdr);
        // int index = _d->pMainLayoutSplitter->indexOf(sdr);
        // Q_ASSERT(index != -1);
        // // 解构子容器
        // if(sdr->_d->mEleWidgetList.size() == 1)
        // {
        //     QWidget* uniqueWidget = sdr->_d->mEleWidgetList.at(0);
        //     _d->pMainLayoutSplitter->replaceWidget(index, uniqueWidget);

        //     sdr->deleteLater();
        // }
    }
private:
    QSplitter* pMainLayoutSplitter = nullptr;
    QList<QWidget*> mEleWidgetList;

    TabContainer* _q = nullptr;
    TabContainer* pParentContainer = nullptr;
    friend class TabContainer;
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
            from->removeTab(index);

            if(_d->mShowMaskOrientation == CENTER)
            {
                this->addTab(page, title);
            }
            else
            {
                emit pageEntered(data, _d->mShowMaskOrientation);
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
    emit pageRemoved();
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
        _d->mEleWidgetList << tab;
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
    _d->mEleWidgetList << tab;
}

// void TabContainer::addContainer(TabContainer* container)
// {
//     _d->pMainLayoutSplitter->addWidget(container);
// }

void TabContainer::clear()
{

}

void TabContainer::onPageRemoved()
{
    // qDebug() << "tab removed" << _d->pMainLayoutSplitter->count();
    TabWidget* from = qobject_cast<TabWidget*>(this->sender());
    Q_ASSERT(from);
    if(from->count() == 0)
    {
        _d->mEleWidgetList.removeOne(from);
        from->deleteLater();
    }
}

void TabContainer::onPageEntered(const TabMoveMimeData* data, TabWidget::Orientations ori)
{
    // qDebug() << "tab entered" << title << ori;
    QWidget* sdr = qobject_cast<QWidget*>(this->sender());
    int index = _d->pMainLayoutSplitter->indexOf(sdr);
    Q_ASSERT(index != -1);
    // index = index == -1 ? 0 : index;

    int orientation = (ori == TabWidget::LEFT || ori == TabWidget::RIGHT) ? Qt::Horizontal : Qt::Vertical;
    if(orientation == _d->pMainLayoutSplitter->orientation())
    {
        TabWidget* newTab = _d->createTabWidget();
        newTab->addTab(data->page(), data->title());
        _d->pMainLayoutSplitter->insertWidget((ori == TabWidget::LEFT || ori == TabWidget::TOP) ? index : index + 1, newTab);
        return;
    }

    // if(_d->pMainLayoutSplitter->count() == 1)
    // {
    //     _d->pMainLayoutSplitter->setOrientation((Qt::Orientation)orientation);
    // }

    // TabContainer* newContainer = new TabContainer;
    // newContainer->_d->pParentContainer = this;
    // // connect(newContainer, &TabContainer::elememtCountChanged, this, &TabContainer::onElementCountChanged);

    // newContainer->_d->pMainLayoutSplitter->setOrientation((Qt::Orientation)orientation);

    // QWidget* oldWidget = _d->pMainLayoutSplitter->replaceWidget(index, newContainer);
    // Q_ASSERT(sdr == oldWidget);
    // this->disconnect(oldWidget);
    // newContainer->_d->connectTabWidget(qobject_cast<TabWidget*>(oldWidget));

    // TabWidget* newTab = newContainer->_d->createTabWidget();
    // newTab->addTab(data->page(), data->title());
    // if((ori == TabWidget::LEFT || ori == TabWidget::TOP))
    // {
    //     newContainer->_d->pMainLayoutSplitter->addWidget(newTab);
    //     newContainer->_d->pMainLayoutSplitter->addWidget(oldWidget);
    //     newContainer->_d->mEleWidgetList << newTab << oldWidget;
    // }
    // else
    // {
    //     newContainer->_d->pMainLayoutSplitter->addWidget(oldWidget);
    //     newContainer->_d->pMainLayoutSplitter->addWidget(newTab);
    //     newContainer->_d->mEleWidgetList << oldWidget << newTab;
    // }

    // // 重构结构
    // _d->rebuildStructure();
    // if(data->container() != this)
    // {
    //     data->container()->_d->rebuildStructure();
    // }
}

void TabContainer::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    p.setPen(Qt::red);
    p.setBrush(Qt::transparent);
    p.drawRect(this->rect().marginsRemoved(QMargins(2,2,2,2)));
}
/************************* ⬆︎ TabContainer ⬆︎ *************************/

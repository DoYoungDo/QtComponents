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
const int INDICATOR_RECT_WIDTH = 2; // px
const int INDICATOR_RECT_WIDTH_HALF = 1; // px
// const char* TYPE_ORIENTATION = "type.orientation";
QColor invertColor(const QColor &color) {
    return QColor(
        255 - color.red(),
        255 - color.green(),
        255 - color.blue(),
        color.alpha()  // 保留透明度
        );
}
}

class TabMoveMimeDataPrivate{
    TabMoveMimeDataPrivate(TabMoveMimeData* q):_q(q){}
    ~TabMoveMimeDataPrivate()
    {
        mDatas.clear();
    }

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

TabMoveMimeData::~TabMoveMimeData()
{
    if(_d)
    {
        delete _d;
        _d = nullptr;
    }
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
/************************* ⬆︎ TabMoveMimeData ⬆︎ *************************/

class TabBarPrivate{
    enum Orientations{
        LEFT,
        RIGHT,
    };
    TabBarPrivate(TabBar* q, TabWidget* tab):_q(q),pTabWidget(tab){}

    void showIndicator(const TabMoveMimeData* data, QPointF pos)
    {
        // qDebug() << "show indicator";
        QPoint p = pos.toPoint();
        int index = _q->tabAt(p);
        // qDebug() << "cur index" << index;
        if(index == -1)
        {
            return;
        }

        int currentIndex = _q->currentIndex();
        if(currentIndex == index && data->fromTabWidget() == pTabWidget)
        {
            canDrop = false;
            _q->repaint();
            return;
        }

        canDrop = true;

        QRect tabRect = _q->tabRect(index);
        QPoint center = tabRect.center();

        QSize indicatorSize = QSize(INDICATOR_RECT_WIDTH, tabRect.height());

        if(p.x() <= center.x())
        {
            indicatorRect = QRect(tabRect.topLeft(), indicatorSize);
            orientations = LEFT;
            targetIndex = index;
        }
        else
        {
            indicatorRect = QRect(QPoint(tabRect.right() - INDICATOR_RECT_WIDTH, tabRect.top()), indicatorSize);
            orientations = RIGHT;
            targetIndex = index + 1;
        }

        // qDebug() << "canDrop" << canDrop << "targetIndex" << targetIndex << "ori" << orientations;

        targetIndex = targetIndex < 0 ? 0 : targetIndex >= _q->count() ? _q->count() : targetIndex;

        _q->repaint();
    }
    void hideIndicator()
    {
        canDrop = false;
        _q->repaint();
    }
private:
    bool isMousePressed = false;
    QPoint pressedPos;

    bool canDrop = false;
    QRect indicatorRect;
    int targetIndex = -1;
    Orientations orientations;

    TabBar* _q;
    friend class TabBar;
    TabWidget* pTabWidget = nullptr;
};
/************************* ⬆︎ TabBarPrivate ⬆︎ *************************/

class TabWidgetPrivate{
    TabWidgetPrivate(TabWidget* q);

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

        _q->repaint();
    }
    void hideTabMask()
    {
        if(!mShowMask)
        {
            return;
        }

        mShowMask = false;
        _q->repaint();
    }

    void addPage(QWidget* page, const QString& title, TabWidget::Orientations ori);

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
    friend class TabBar;
    friend class TabContainer;
    friend class TabContainerPrivate;
};
/************************* ⬆︎ TabWidgetPrivate ⬆︎ *************************/

class TabSplitterPrivate
{
    TabSplitterPrivate(TabSplitter* q):_q(q){}
    ~TabSplitterPrivate()
    {
        mValidWidgets.clear();
    }

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
    ~TabContainerPrivate()
    {
        pMainLayoutSplitter = nullptr;
        pParentContainer = nullptr;
        _q = nullptr;
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
        tabWidget->setTabsClosable(mTabsClosable);
        return tabWidget;
    }

    TabContainer* createContainer()
    {
        TabContainer* container = new TabContainer();
        container->setTabsClosable(mTabsClosable);
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
        // 检查当前容器是否有父容器，若有则进行结构重建操作
        if(pParentContainer)
        {
            // 获取父容器主布局分割器中的部件数量
            int count = pParentContainer->_d->pMainLayoutSplitter->widgetCount();

            // 存储父容器主布局分割器中的 TabContainer 部件
            QList<TabContainer*> containers;
            // 存储父容器主布局分割器中的 TabWidget 部件
            QList<TabWidget*> tabs;

            // 遍历父容器主布局分割器中的所有部件，将其分类到 containers 和 tabs 列表中
            for(int i = 0; i < count; ++i)
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

            // 遍历所有的 TabContainer 部件，根据不同情况调整布局结构
            for(TabContainer* container : containers)
            {
                // 情况 1: 当前 TabContainer 为空，从父容器中移除
                if(container->_d->pMainLayoutSplitter->isEmpty())
                {
                    pParentContainer->_d->pMainLayoutSplitter->removeWidget(container);
                    continue;
                }
                // 情况 2: 父容器主布局分割器只有一个部件
                else if(pParentContainer->_d->isAnyOrirentaion())
                {
                    // 同步父容器主布局分割器的方向
                    pParentContainer->_d->pMainLayoutSplitter->setOrientation(container->_d->pMainLayoutSplitter->orientation());
                    // 获取当前 TabContainer 中的所有部件
                    QList<QWidget*> widgets = container->_d->pMainLayoutSplitter->takeAll();
                    // 移除当前 TabContainer
                    pParentContainer->_d->pMainLayoutSplitter->removeWidget(container);
                    // 将部件添加到父容器主布局分割器中
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
                // 情况 3: 当前 TabContainer 主布局分割器只有一个部件
                else if(container->_d->isAnyOrirentaion())
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
                    // 替换父容器主布局分割器中的部件
                    pParentContainer->_d->pMainLayoutSplitter->replaceWidget(pParentContainer->_d->pMainLayoutSplitter->indexOf(container), w);
                    container->deleteLater();
                    continue;
                }
                // 情况 4: 当前 TabContainer 与父容器主布局分割器方向相同
                else if(container->_d->pMainLayoutSplitter->orientation() == pParentContainer->_d->pMainLayoutSplitter->orientation())
                {
                    QList<QWidget*> widgets = container->_d->pMainLayoutSplitter->takeAll();
                    Q_ASSERT(!widgets.isEmpty());

                    QWidget* first = widgets.takeAt(0);
                    if(isContainer(first))
                    {
                        qobject_cast<TabContainer*>(first)->_d->pParentContainer = pParentContainer;
                    }
                    else
                    {
                        qobject_cast<TabWidget*>(first)->_d->pContainer = pParentContainer;
                    }

                    int index = pParentContainer->_d->pMainLayoutSplitter->indexOf(container);
                    // 替换第一个部件
                    pParentContainer->_d->pMainLayoutSplitter->replaceWidget(index, first);
                    // 插入剩余部件
                    for(int i = 0; i < widgets.size(); ++i)
                    {
                        QWidget* w = widgets.at(i);
                        if(isContainer(w))
                        {
                            qobject_cast<TabContainer*>(w)->_d->pParentContainer = pParentContainer;
                        }
                        else
                        {
                            qobject_cast<TabWidget*>(w)->_d->pContainer = pParentContainer;
                        }
                        pParentContainer->_d->pMainLayoutSplitter->insertWidget(index + i + 1, w);
                    }
                    container->deleteLater();
                    continue;
                }
            }

            // 递归调用父容器的 rebuildStructure 方法，继续重建结构
            pParentContainer->_d->rebuildStructure();
        }
    }


private:
    TabSplitter* pMainLayoutSplitter = nullptr;
    TabContainer* pParentContainer = nullptr;
    bool mTabsClosable = false;

    TabContainer* _q = nullptr;
    friend class TabContainer;
    friend class TabBar;
    friend class TabWidget;
    friend class TabWidgetPrivate;
};
/************************* ⬆︎ TabContainerPrivate ⬆︎ *************************/

TabWidgetPrivate::TabWidgetPrivate(TabWidget* q):_q(q)
{
    pTabbar = new TabBar(_q, _q);
    _q->connect(pTabbar, &TabBar::tabDraged, _q, &TabWidget::onTabDraged);
    _q->connect(pTabbar, &TabBar::tabCloseRequested, _q, [this](int index){
        QWidget* w = _q->widget(index);
        Q_ASSERT(w);
        QString title = _q->tabText(index);
        emit pContainer->pageCloseRequested(w, title);
    });

    _q->setTabBar(pTabbar);
}

void TabWidgetPrivate::addPage(QWidget* page, const QString& title, TabWidget::Orientations ori)
{
    if(ori == TabWidget::CENTER)
    {
        _q->addTab(page, title);
    }
    else
    {
        TabContainer* currentContainer = pContainer;
        TabSplitter* layout = currentContainer->_d->pMainLayoutSplitter;
        int index = layout->indexOf(_q);
        Q_ASSERT(index != -1);

        int orientation = (ori == TabWidget::LEFT || ori == TabWidget::RIGHT) ? Qt::Horizontal : Qt::Vertical;
        if(orientation == layout->orientation())
        {
SAME_ORI:
            TabWidget* newTab = currentContainer->_d->createTabWidget();
            newTab->addTab(page, title);
            layout->insertWidget((ori == TabWidget::LEFT || ori == TabWidget::TOP) ? index : index + 1, newTab);
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
            Q_ASSERT(_q == oldWidget);
            pContainer = newContainer;

            TabWidget* newTab = newContainer->_d->createTabWidget();
            newTab->addTab(page, title);

            if((ori == TabWidget::LEFT || ori == TabWidget::TOP))
            {
                *newContainer->_d->pMainLayoutSplitter << newTab << oldWidget;
            }
            else
            {
                *newContainer->_d->pMainLayoutSplitter << oldWidget << newTab;
            }
        }
    }
}
/************************* ⬆︎ TabWidgetPrivate impletment ⬆︎ *************************/

TabBar::TabBar(TabWidget* tabwidget, QWidget* parent)
    :QTabBar(parent)
    ,_d(new TabBarPrivate(this, tabwidget))
{
    this->setAcceptDrops(true);
}

TabBar::~TabBar()
{
    if(_d)
    {
        delete _d;
        _d = nullptr;
    }
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

void TabBar::dragEnterEvent(QDragEnterEvent* event)
{
    const TabMoveMimeData *data = qobject_cast<const TabMoveMimeData *>(event->mimeData());
    // qDebug() << "enter data" << (void*)data;
    if (data)
    {
        _d->showIndicator(data, event->position());
        event->accept();
    }
    QTabBar::dragEnterEvent(event);
}

void TabBar::dragMoveEvent(QDragMoveEvent* event)
{
    const TabMoveMimeData *data = qobject_cast<const TabMoveMimeData *>(event->mimeData());
    if (data)
    {
        _d->showIndicator(data, event->position());
    }
    QTabBar::dragMoveEvent(event);
}

void TabBar::dragLeaveEvent(QDragLeaveEvent* event)
{
    _d->hideIndicator();

    QTabBar::dragLeaveEvent(event);
}

void TabBar::dropEvent(QDropEvent* event)
{
    const TabMoveMimeData *data = qobject_cast<const TabMoveMimeData *>(event->mimeData());
    if(data)
    {
        if(_d->canDrop)
        {
            _d->hideIndicator();

            QWidget* page = data->page();
            TabWidget* from = data->fromTabWidget();

            int index = from->indexOf(page);
            QString title = from->tabText(index);
            TabContainer* fromContainer = from->_d->pContainer;

            if(_d->pTabWidget == from)
            {
                QWidget* targetPage = from->widget(_d->targetIndex);
                from->removeTab(index);
                int newTargetIndex = from->indexOf(targetPage);
                int newIndex = from->insertTab(newTargetIndex, page, title);
                from->setCurrentIndex(newIndex);
                this->setCurrentIndex(newIndex);
            }
            else
            {
                from->removeTab(index);

                int newIndex = _d->pTabWidget->insertTab(_d->targetIndex, page, title);
                _d->pTabWidget->setCurrentIndex(newIndex);
                this->setCurrentIndex(newIndex);

                fromContainer->_d->rebuildStructure();
                _d->pTabWidget->_d->pContainer->_d->rebuildStructure();
            }

            this->repaint();
            event->acceptProposedAction();
            return;
        }
    }
    QTabBar::dropEvent(event);
}

void TabBar::paintEvent(QPaintEvent* event)
{
    QTabBar::paintEvent(event);

    if(_d->canDrop)
    {
        QPainter p(this);
        p.setPen(Qt::transparent);
        p.setBrush(invertColor(this->palette().brush(QPalette::Window).color()));
        p.drawRect(_d->indicatorRect);
    }
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

TabWidget::~TabWidget()
{
    if(_d)
    {
        delete _d;
        _d = nullptr;
    }
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
    const TabMoveMimeData *data = qobject_cast<const TabMoveMimeData *>(event->mimeData());
    if(data)
    {
        _d->hideTabMask();
        // qDebug() << "get data";

        if(_d->canTrop(data, event->position()))
        {
            QWidget* page = data->page();
            TabWidget* from = data->fromTabWidget();

            int index = from->indexOf(page);
            QString title = from->tabText(index);
            TabContainer* fromContainer = from->_d->pContainer;
            from->removeTab(index);

            _d->addPage(page, title, _d->mShowMaskOrientation);

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
        p.setBrush(invertColor(this->palette().brush(QPalette::Window).color()));
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
    // qDebug() << "tab draged" << index;
    this->setCurrentIndex(index);
    _d->pTabbar->setCurrentIndex(index);

    TabMoveMimeData* mimeData = new TabMoveMimeData;
    mimeData->setPage(this->widget(index));
    mimeData->setTitle(this->tabText(index));
    mimeData->setFromTabWidget(this);

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

TabSplitter::~TabSplitter()
{
    if(_d)
    {
        delete _d;
        _d = nullptr;
    }
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

QList<QWidget*> TabSplitter::allWidget()
{
    return _d->mValidWidgets;
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
    connect(this, &TabContainer::pageCloseRequested, this, [this](QWidget* page, const QString& label){
        qDebug() << "container close";
        if(_d->pParentContainer)
        {
            qDebug() << "has parent";
            emit _d->pParentContainer->pageCloseRequested(page, label);
        }
    });
}

TabContainer::~TabContainer()
{
    if(_d)
    {
        delete _d;
        _d = nullptr;
    }
}

void TabContainer::addPage(QWidget* page, const QString& label)
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
            qobject_cast<TabContainer*>(ele)->addPage(page, label);
        }
        else
        {
            qobject_cast<TabWidget*>(ele)->addTab(page, label);
        }
    }
}

void TabContainer::addPage(QWidget* page, const QString& label, bool split)
{
    if(!split)
    {
        addPage(page, label);
        return;
    }

    TabWidget* tab = _d->createTabWidget();
    tab->addTab(page, label);
    _d->pMainLayoutSplitter->addWidget(tab);
}

void TabContainer::addPage(QWidget* page, const QString& label, QWidget* splitFrom, TabWidget::Orientations ori)
{
    QList<TabWidget*> tabWidgets = this->findChildren<TabWidget*>();
    for(TabWidget* w: tabWidgets)
    {
        if(w->indexOf(splitFrom) != -1)
        {
            w->_d->addPage(page, label, ori);
            return;
        }
    }
}

void TabContainer::removePage(QWidget* page)
{
    QList<TabWidget*> tabWidgets = this->findChildren<TabWidget*>();
    for(TabWidget* w: tabWidgets)
    {
        int index = w->indexOf(page);
        if(index != -1)
        {
            w->removeTab(index);
            w->_d->pContainer->_d->rebuildStructure();
            return;
        }
    }
}

void TabContainer::removeAll()
{
    _d->pMainLayoutSplitter->removeAll();
}

void TabContainer::setTabsClosable(bool closeable)
{
    if(_d->mTabsClosable == closeable)
    {
        return;
    }

    _d->mTabsClosable = closeable;

    QList<TabWidget*> tabWidgets = this->findChildren<TabWidget*>();
    for(TabWidget* w: tabWidgets)
    {
        w->setTabsClosable(_d->mTabsClosable);
    }
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

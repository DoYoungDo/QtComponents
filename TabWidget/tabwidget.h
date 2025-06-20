#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QEvent>
#include <QMimeData>
#include <QPainter>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>

class TabWidget;
class TabContainer;

class TabMoveMimeData : public QMimeData{
    Q_OBJECT
public:
    TabMoveMimeData();
    ~TabMoveMimeData();
public:
    void setPage(QWidget* page);
    QWidget* page() const;
    void setTitle(const QString& title);
    QString title() const;
    void setFromTabWidget(TabWidget* from);
    TabWidget* fromTabWidget() const;

private:
    class TabMoveMimeDataPrivate* _d;
    friend class TabMoveMimeDataPrivate;
};

class TabBar: public QTabBar{
    Q_OBJECT
private:
    TabBar(TabWidget* tabwidget, QWidget *parent = nullptr);
    ~TabBar();

signals:
    void tabDraged(int index);
protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    class TabBarPrivate* _d;
    friend class TabBarPrivate;
    friend class TabWidgetPrivate;
};

class TabWidget : public QTabWidget{
    Q_OBJECT
public:
    enum Orientations{
        LEFT,
        TOP,
        RIGHT,
        BOTTOM,
        CENTER
    };
private:
    TabWidget(TabContainer* container, QWidget *parent = nullptr);
    ~TabWidget();

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void tabInserted(int index) override;
    virtual void tabRemoved(int index) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void pageEntered(const TabMoveMimeData* data, Orientations ori);
private slots:
    void onTabDraged(int index);
private:
    class TabWidgetPrivate* _d;
    friend class TabWidgetPrivate;
    friend class TabBar;
    friend class TabContainer;
    friend class TabContainerPrivate;
};

class TabSplitter : public QSplitter
{
private:
    explicit TabSplitter(QWidget* parent = nullptr);
    ~TabSplitter();

public:
    void addWidget(QWidget *w);
    void insertWidget(int index, QWidget *w);
    QWidget *replaceWidget(int index, QWidget *w);
    int indexOf(QWidget* w);
    QWidget *widget(int index);
    QList<QWidget*> allWidget();
    QWidget* tabkeWidget(int index);
    QList<QWidget*> takeAll();
    void removeWidget(QWidget* w);
    void removeWidget(int index);
    void removeAll();
    int widgetCount();
    bool isEmpty();
    QWidget* first();
    QWidget* last();

    TabSplitter& operator<<(QWidget* w);
private:
    class TabSplitterPrivate* _d;
    friend class TabSplitterPrivate;
    friend class TabContainerPrivate;
};

class TabContainer : public QWidget
{
    Q_OBJECT
public:
    TabContainer(QWidget* parent = nullptr);
    ~TabContainer();

    void addPage(QWidget *page, const QString &label);
    void addPage(QWidget *page, const QString &label, bool split);
    void addPage(QWidget *page, const QString &label, QWidget* splitFrom, TabWidget::Orientations ori);
    void removePage(QWidget* page);
    void removeAll();

    void setTabsClosable(bool closeable);

    void setPageLabel(QWidget* page, const QString& label);

signals:
    void pageCloseRequested(QWidget* page, const QString& label);
private:
    class TabContainerPrivate* _d;
    friend class TabContainerPrivate;
    friend class TabBar;
    friend class TabWidget;
    friend class TabWidgetPrivate;

// #define TAB_TEST
#ifdef TAB_TEST
public:
    void print();

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent* event) override;
#endif
};

#endif // TABWIDGET_H

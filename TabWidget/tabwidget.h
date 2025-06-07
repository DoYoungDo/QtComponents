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
private:
    enum Orientations{
        LEFT,
        TOP,
        RIGHT,
        BOTTOM,
        CENTER
    };
    TabWidget(TabContainer* container, QWidget *parent = nullptr);

protected:
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void tabInserted(int index) override;
    virtual void tabRemoved(int index) override;

signals:
    void pageRemoved();
    void pageEntered(const TabMoveMimeData* data, Orientations ori);
private slots:
    void onTabDraged(int index);
private:
    class TabWidgetPrivate* _d;
    friend class TabWidgetPrivate;
    friend class TabBar;
    friend class TabContainerPrivate;
};

class TabSplitter : public QSplitter
{
private:
    explicit TabSplitter(QWidget* parent = nullptr);

public:
    void addWidget(QWidget *w);
    void insertWidget(int index, QWidget *w);
    QWidget *replaceWidget(int index, QWidget *w);
    int indexOf(QWidget* w);
    QWidget *widget(int index);
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

    void addPage(QWidget *page, const QString &label);
    void addPage(QWidget *page, const QString &label, bool split);
    void removeAll();
private:
    class TabContainerPrivate* _d;
    friend class TabContainerPrivate;
    friend class TabBar;
    friend class TabWidget;

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

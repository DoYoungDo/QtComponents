#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QEvent>
#include <QMimeData>
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
    void setContainer(TabContainer* container);
    TabContainer* container() const;

private:
    class TabMoveMimeDataPrivate* _d;
    friend TabMoveMimeDataPrivate;
};

class TabBar: public QTabBar{
    Q_OBJECT
public:
    TabBar(QWidget *parent = nullptr);

signals:
    void tabDraged(int index);
protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
    class TabBarPrivate* _d;
    friend TabBarPrivate;
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
    friend TabWidgetPrivate;
};

class TabContainer : public QWidget
{
    Q_OBJECT
public:
    TabContainer(QWidget* parent = nullptr);

    void addTab(QWidget *page, const QString &label);
    void addTab(QWidget *page, const QString &label, bool split);
    void clear();

private slots:
    void onPageRemoved();
    void onPageEntered(const TabMoveMimeData* data, TabWidget::Orientations ori);
private:
    class TabContainerPrivate* _d;
    friend TabContainerPrivate;

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent* event) override;
};

#endif // TABWIDGET_H

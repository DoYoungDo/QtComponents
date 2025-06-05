#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QObject>
#include <QStyleOption>
class QPainter;

class LoadingWidget : public QObject
{
    Q_OBJECT
public:
    explicit LoadingWidget(QObject *parent = nullptr);
    ~LoadingWidget();

    void startLoading(QWidget* w);
    void startLoading(QWidget* w, bool disable);
    void startLoading(QWidget* w, bool disable, bool expanding);
    void startLoading(QWidget* w, bool disable, int radius);
    void stopLoading(QWidget* w);

protected:
    // 创建绘制时的样式选项，整个加载期间所有绘制只调用一次
    virtual QStyleOption* createOption();
    // 初始化样式，加载期间每次绘制都会调用
    virtual void initStyleOption(QStyleOption* opt, QWidget* w);
    // 绘制加载
    virtual void drawLoading(QPainter* p, QStyleOption* opt);
private:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void timerEvent(QTimerEvent* event) override;

private:
    class LoadingWidgetPrivate* _d;
    friend class LoadingWidgetPrivate;
};

// 通过绘制图片显示加载
class PictureLoadingWidget : public LoadingWidget
{
public:
    PictureLoadingWidget(QObject *parent = nullptr);
    // LoadingWidget interface
protected:
    virtual void drawLoading(QPainter* p, QStyleOption* opt) override;
};

#endif // LOADINGWIDGET_H

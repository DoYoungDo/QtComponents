#ifndef CONFIGURATIONDIAPLAYER_H
#define CONFIGURATIONDIAPLAYER_H

#include <QObject>
#include <QFrame>
#include <QStandardItem>
#include <QListWidget>
#include <QLineEdit>
#include <QTimer>

class QTreeView;
class QLabel;
class QPushButton;

class ConfigurationSearchBar;
class ConfigurationListWidget;
class ConfigurationListItemWidget;
class ItemWidgetContainer;
class ConfigurationTitleDiaplayer;

enum SchemaPropertyType
{
    STRING,
    NUMBER,
    BOOLEAN,
    // ARRAY,
    // OBJECT,
};
struct SchemaItem{
    SchemaPropertyType type;
};
struct SchemaEnumItem{
    QString description;
};
typedef QString ConfigurationId;
struct SchemaProperty
{
    ConfigurationId id;
    QString description;
    SchemaPropertyType type;
    QVariant defaultValue;
    QVariant value;
    QVariantList enumValue;
    QList<SchemaEnumItem> enumItems;
    QList<SchemaItem> items;
};
struct SchemaConfiguration
{
    QString title;
    QMap<ConfigurationId,SchemaProperty> properties;
    QList<SchemaConfiguration> configuration;
};

/**
 * @brief The Searchable class
 * 接口
 * 抽象可搜索对象的接口，如果对象可被搜索，需要实现此接口
 */
class Searchable
{
public:
    /**
     * @brief search
     * 开始搜索控件
     * @param text
     * 过滤文本条件
     * @return true 搜索到  false 没有搜索到
     */
    virtual bool search(const QString& text) = 0;
    /**
     * @brief canclelSearch
     * 取消搜索
     */
    virtual void cancelSearch() = 0;
public:
    QString createHighlightText(const QString& text, const QString& backgroundColor, const QString& foregroundColor);
    QString highlightText(const QString& text, const QString& match, const QString& backgroundColor, const QString& foregroundColor);
};

/**
 * @brief The ConfigurationDiaplayer class
 * 对外暴露的唯一类，配置项显示、编辑控件
 */
class ConfigurationDiaplayer : public QFrame, public Searchable
{
    Q_OBJECT
public:
    ConfigurationDiaplayer(QWidget* parent = nullptr);
    ConfigurationDiaplayer(QList<SchemaConfiguration> configs, QWidget* parent = nullptr);
    ~ConfigurationDiaplayer();

    void reloadConfigurations(QList<SchemaConfiguration> configs);

    void addConfiguration(SchemaConfiguration config);
    void addConfiguration(SchemaConfiguration config, ConfigurationTitleDiaplayer* parent);

    /**
     * @brief setSearchResponseInterval
     * 设置搜索自动响应间隔，单位 ms
     * 搜索栏输入时，自动响应时间，回车或失去焦点还是立即响应
     * @param interval 默认 500
     */
    void setSearchAutoResponseInterval(unsigned int interval = 500);
    /**
     * @brief setSearchFilterButtonHide
     * 设置搜索栏过滤按钮是否隐藏
     * @param hide
     */
    void setSearchFilterButtonHide(bool hide);
    /**
     * @brief setSearchHighlightBackgroundColor
     * 设置搜索高亮背景颜色
     * @param color
     */
    void setSearchHighlightBackgroundColor(const QString& color);
    /**
     * @brief setSearchHighlightForegroundColor
     * 设置搜索高亮前景颜色
     * @param color
     */
    void setSearchHighlightForegroundColor(const QString& color);
    /**
     * @brief setInputWarningText
     * 设置输入警告文本
     * @param warn
     */
    void setInputWarningText(const QString& warn);
    /**
     * @brief setInputWarningTextBackgroundColor
     * 设置输入警告文本背景颜色
     * @param color
     */
    void setInputWarningTextBackgroundColor(const QString& color);
    /**
     * @brief setInputWarningTextForegroundColor
     * 设置输入警告文本前景颜色
     * @param color
     */
    void setInputWarningTextForegroundColor(const QString& color);

    static SchemaProperty propertyFromJson(QJsonObject property, const QString id);
    static bool isValidConfigurationFromJson(QJsonObject config);
    static SchemaConfiguration configurationFromJson(QJsonObject config);
    static QList<SchemaConfiguration> configurationFromJsonArray(QJsonArray configs);
signals:
    void propertyValueChanged(SchemaProperty property, const QVariant& oldValue, const QVariant& newValue);

public slots:
    void updateCurrentTitle(QListWidgetItem *item);

    // Searchable interface
public:
    virtual bool search(const QString& text) override;
    virtual void cancelSearch() override;

private:
    void setupUI();
    void fillView();
private:
    ConfigurationSearchBar* pSearchInput;
    QTreeView* pTitleView;
    QStandardItemModel* pTitleViewModel;
    ConfigurationListWidget* pConfigView;
    class ConfigurationDiaplayerPrivate* _d;
};

/************************* 以下为内部使用类，外部不应使用 *************************/

/**
 * @brief The ConfigurationSearchBar class
 * 自定义搜索栏控件，提供搜索输入，内部实现了防抖的边输入边搜索
 */
class ConfigurationSearchBar : public QLineEdit
{
    Q_OBJECT
private:
    friend class ConfigurationDiaplayer;
    ConfigurationSearchBar(ConfigurationDiaplayerPrivate* data, QWidget* parent = nullptr);

    void addToolButton(QList<QPushButton*> buttons);
    void clearToolButton();

signals:
    void cancelSearch();
private:
    std::function<void()> updateInterval;
    ConfigurationDiaplayerPrivate* pData;
};

class ConfigurationTitleModelItem : public QStandardItem
{
private:
    friend class ConfigurationDiaplayer;
    friend class ConfigurationTitleDiaplayer;
    ConfigurationTitleModelItem(const QString &text):QStandardItem(text){}
    void setTitleDisplayer(ConfigurationTitleDiaplayer* titleDisplayer){pTitleDisplayer = titleDisplayer;}
    ConfigurationTitleDiaplayer* getTitleDisplayer(){return pTitleDisplayer;}
private:
    ConfigurationTitleDiaplayer* pTitleDisplayer;
};

/**
 * @brief The ConfigurationListWidget class
 * 自定义列表控件，封装了对列表进行搜索的功能
 */
class ConfigurationListWidget : public QListWidget, public Searchable
{
private:
    friend class ConfigurationDiaplayer;
    explicit ConfigurationListWidget(class ConfigurationDiaplayerPrivate* data, QWidget *parent = nullptr)
        :QListWidget(parent)
        ,pData(data)
    {
        // this->setViewportMargins(0,50,0,0);
    }

public:
    void addCustomListItemWidget(ConfigurationListItemWidget* w);

    // Searchable interface
public:
    virtual bool search(const QString& text) override;
    virtual void cancelSearch() override;

private:
    QWidget* createContainer(QWidget* content);

private:
    class ConfigurationDiaplayerPrivate* pData;
};

/**
 * @brief The ConfigurationListItemWidget class
 * 自定义列表控件项控件
 */
class ConfigurationListItemWidget : public QFrame, public Searchable
{
    Q_OBJECT
public:
    ConfigurationListItemWidget(class ConfigurationDiaplayerPrivate* data, QWidget* parent = nullptr);

public:
    virtual QListWidgetItem* getOrCreateListItem();
protected:
    QListWidgetItem* pListItem = nullptr;
    class ConfigurationDiaplayerPrivate* pData;
};

/**
 * @brief The ConfigurationTitleDiaplayer class
 * 配置项标题显示器控件
 */
class ConfigurationTitleDiaplayer : public ConfigurationListItemWidget
{
    Q_OBJECT
private:
    friend class ConfigurationDiaplayer;
    ConfigurationTitleDiaplayer(const QString& title, class ConfigurationDiaplayerPrivate* data, ConfigurationTitleDiaplayer* parent = nullptr, QWidget* parentW = nullptr);

    void setModelItem(ConfigurationTitleModelItem* item){pModelItem = item;}
    ConfigurationTitleModelItem* getModelItem(){return pModelItem;}

    // Searchable interface
public:
    virtual bool search(const QString& text) override;
    virtual void cancelSearch() override;

protected:
    void addChild(ConfigurationTitleDiaplayer* child);

private:
    int fontSize();
    int level();
private:
    friend class ConfigurationPropertyDiaplayer;
    QString mTitle;
    ConfigurationTitleDiaplayer* pParent;
    QList<ConfigurationTitleDiaplayer*> mChildren;
    ConfigurationTitleModelItem* pModelItem;
};

/**
 * @brief The ConfigurationPropertyDiaplayer class
 * 配置项属性显示器控件，基类，所有类型的属性都实现此接口
 */
class ConfigurationPropertyDiaplayer : public ConfigurationListItemWidget
{
    Q_OBJECT
public:
    ConfigurationPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, class ConfigurationDiaplayerPrivate* data, QWidget* parent = nullptr);
    virtual ~ConfigurationPropertyDiaplayer() = default;

    void print();
    ConfigurationTitleDiaplayer* title(){return pTitle;}

signals:
    void propertyValueChanged(SchemaProperty property, const QVariant& oldValue, const QVariant& newValue);
    void focusIn();

protected:
    QLabel* createIdLabel();
    QLabel* createDescriptionLabel();

protected:
    SchemaProperty mProperty;
    ConfigurationTitleDiaplayer* pTitle;

public:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

class StringPropertyDiaplayer : public ConfigurationPropertyDiaplayer
{
    Q_OBJECT
private:
    friend class ConfigurationPropertyDiaplayerCreator;
    StringPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, class ConfigurationDiaplayerPrivate* data, QWidget* parent = nullptr);
    virtual ~StringPropertyDiaplayer(){}

    // Searchable interface
public:
    virtual bool search(const QString& text) override;
    virtual void cancelSearch() override;
};

class NumberPropertyDiaplayer : public ConfigurationPropertyDiaplayer
{
    Q_OBJECT
private:
    friend class ConfigurationPropertyDiaplayerCreator;
    NumberPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, class ConfigurationDiaplayerPrivate* data, QWidget* parent = nullptr);
    virtual ~NumberPropertyDiaplayer(){}

    // Searchable interface
public:
    virtual bool search(const QString& text) override;
    virtual void cancelSearch() override;
};

class BooleanPropertyDiaplayer : public ConfigurationPropertyDiaplayer
{
    Q_OBJECT
private:
    friend class ConfigurationPropertyDiaplayerCreator;
    BooleanPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, class ConfigurationDiaplayerPrivate* data, QWidget* parent = nullptr);
    virtual ~BooleanPropertyDiaplayer(){}

    // Searchable interface
public:
    virtual bool search(const QString& text) override;
    virtual void cancelSearch() override;

    // QObject interface
public:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // CONFIGURATIONDIAPLAYER_H

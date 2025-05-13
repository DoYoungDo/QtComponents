#include "configurationdiaplayer.h"

#include <QApplication>
#include <QCheckBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>
#include <QScrollBar>
#include <QScroller>
#include <QComboBox>
#include <QPushButton>
#include <QWidgetAction>
#include <QPointer>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

class ConfigurationPropertyDiaplayerCreator
{
private:
    ConfigurationPropertyDiaplayerCreator();

public:
    static ConfigurationPropertyDiaplayerCreator* instance();
    ConfigurationPropertyDiaplayer* createDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data);

    bool isValid(QJsonObject property);
    SchemaProperty propertyFromJson(QJsonObject property, const QString& id);
private:
    typedef std::function<bool(SchemaProperty property)> PropertyChecher;
    QMap<SchemaPropertyType,PropertyChecher> mPropertyCheckers;

    typedef std::function<bool(QJsonObject property)> PropertyJsonChecher;
    QMap<SchemaPropertyType,PropertyJsonChecher> mPropertyJsonCheckers;

    typedef std::function<ConfigurationPropertyDiaplayer*(SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data)> PropertyCreator;
    QMap<SchemaPropertyType,PropertyCreator> mPropertyCreators;

    typedef std::function<SchemaProperty(QJsonObject property, const QString& id)> PropertyJsonCreator;
    QMap<SchemaPropertyType,PropertyJsonCreator> mPropertyJsonCreators;
};

ConfigurationPropertyDiaplayerCreator::ConfigurationPropertyDiaplayerCreator()
{
    auto stringChecker = [](SchemaProperty property)->bool{
        return !property.id.isEmpty() && property.type == STRING;
    };
    auto stringJsonChecker = [](QJsonObject property)->bool{
        if(!(property.value("type").toString() == "string"
               && property.value("description").isString()))
        {
            return false;
        }
        if(property.contains("enum"))
        {
            if(!property.value("enum").isArray())
            {
                return false;
            }

            if(!property.contains("enumItems") || !property.value("enumItems").isArray())
            {
                return false;
            }

            for(auto ele : property.value("enumItems").toArray())
            {
                if(!ele.isObject())
                {
                    return false;
                }
                if(!ele.toObject().contains("description"))
                {
                    return false;
                }
            }
        }
        return true;
    };
    auto stringCreator = [](SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data)->ConfigurationPropertyDiaplayer*{
        return new StringPropertyDiaplayer(property, title, data, nullptr);
    };
    auto stringJsonCreator = [](QJsonObject property, const QString& id)->SchemaProperty{
        SchemaProperty p ={
                id,
                property.value("description").toString(),
                STRING,
                property.value("default").toString(""),
                property.value("default").toString(""),
                property.value("enum").toArray().toVariantList()
            };

        if(property.contains("enumItems") && property.value("enumItems").isArray())
        {
            for(auto ele : property.value("enumItems").toArray())
            {
                p.enumItems << SchemaEnumItem{ele.toObject().value("description").toString()};
            }
        }

        return p;
    };

    auto numberChecker = [](SchemaProperty property)->bool{
        return !property.id.isEmpty() && property.type == NUMBER;
    };
    auto numberJsonChecker = [](QJsonObject property)->bool{
        return property.value("type").toString() == "number"
               && property.value("description").isString();
    };
    auto numberCreator = [](SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data)->ConfigurationPropertyDiaplayer*{
        return new NumberPropertyDiaplayer(property, title, data, nullptr);
    };
    auto numberJsonCreator = [](QJsonObject property, const QString& id)->SchemaProperty{
        return SchemaProperty{
            id,
            property.value("description").toString(),
            NUMBER,
            property.value("default").toDouble(0),
        };
    };

    auto booleanChecker = [](SchemaProperty property)->bool{
        return !property.id.isEmpty() && property.type == BOOLEAN;
    };
    auto booleanJsonChecker = [](QJsonObject property)->bool{
        return property.value("type").toString() == "boolean"
               && property.value("description").isString();
    };
    auto booleanCreator = [](SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data)->ConfigurationPropertyDiaplayer*{
        return new BooleanPropertyDiaplayer(property, title, data, nullptr);
    };
    auto booleanJsonCreator = [](QJsonObject property, const QString& id)->SchemaProperty{
        return SchemaProperty{
            id,
            property.value("description").toString(),
            BOOLEAN,
            property.value("default").toBool(false),
        };
    };

    mPropertyCheckers = {
        {SchemaPropertyType::STRING,stringChecker},
        {SchemaPropertyType::NUMBER,numberChecker},
        {SchemaPropertyType::BOOLEAN,booleanChecker}
    };
    mPropertyJsonCheckers = {
        {SchemaPropertyType::STRING,stringJsonChecker},
        {SchemaPropertyType::NUMBER,numberJsonChecker},
        {SchemaPropertyType::BOOLEAN,booleanJsonChecker}
    };
    mPropertyCreators = {
        {SchemaPropertyType::STRING,stringCreator},
        {SchemaPropertyType::NUMBER,numberCreator},
        {SchemaPropertyType::BOOLEAN,booleanCreator}
    };
    mPropertyJsonCreators = {
        {SchemaPropertyType::STRING,stringJsonCreator},
        {SchemaPropertyType::NUMBER,numberJsonCreator},
        {SchemaPropertyType::BOOLEAN,booleanJsonCreator}
    };
}

ConfigurationPropertyDiaplayerCreator* ConfigurationPropertyDiaplayerCreator::instance()
{
    static ConfigurationPropertyDiaplayerCreator* inst = new ConfigurationPropertyDiaplayerCreator();
    return inst;
}

ConfigurationPropertyDiaplayer* ConfigurationPropertyDiaplayerCreator::createDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data)
{
    if(mPropertyCheckers.value(property.type)(property)){
        return mPropertyCreators.value(property.type)(property, title, data);
    }

    return nullptr;
}

bool ConfigurationPropertyDiaplayerCreator::isValid(QJsonObject property)
{
    for(auto value : mPropertyJsonCheckers.values())
    {
        if(value(property))
        {
            return true;
        }
    }
    return false;
}

SchemaProperty ConfigurationPropertyDiaplayerCreator::propertyFromJson(QJsonObject property, const QString& id)
{
    for(auto key : mPropertyJsonCheckers.keys())
    {
        auto value = mPropertyJsonCheckers.value(key);
        if(value(property))
        {
            return mPropertyJsonCreators.value(key)(property, id);
        }
    }
    Q_ASSERT(false);
}

/**************************************************/

QString Searchable::createHighlightText(const QString& text, const QString& backgroundColor, const QString& foregroundColor)
{
    return QString("<font style=\"background:%1; color:%2\">%3</font>").arg(backgroundColor, foregroundColor, text);
}

QString Searchable::highlightText(const QString& text, const QString& match, const QString& backgroundColor, const QString& foregroundColor)
{
    QString newText(text);
    int len = match.size();
    int from = 0;
    int index = -1;
    QList<int> indexList;
    while ((index = newText.indexOf(match, from, Qt::CaseInsensitive)) != -1) {
        indexList << index;
        from += (index + len);
    }
    for(auto i = indexList.size() - 1;i >= 0;--i)
    {
        int indexPos = indexList.at(i);
        newText.replace(indexPos, len, this->createHighlightText(newText.sliced(indexPos, len),backgroundColor,foregroundColor));
    }
    return newText;
}

/**************************************************/
class ConfigurationDiaplayerPrivate
{
public:
    ConfigurationDiaplayerPrivate(ConfigurationDiaplayer* p)
        :_p(p){mConfigs.clear();}

    ConfigurationDiaplayer* _p;
    QList<SchemaConfiguration> mConfigs;
    bool sCanUpdateCurrentTitle = true;
    int mSearchAutoResponseInterval = 500;
    QString mHighlightBackgroundColor = "#c9c6c8";
    QString mHighlightForegroundColor = "#f9f8f7";
    QString mWarningText = "无效的输入";
    QString mWarningTextBackgroundColor = "transparent";
    QString mWarningTextForegroundColor = "red";
};

/**************************************************/

ConfigurationDiaplayer::ConfigurationDiaplayer(QWidget* parent)
    :QFrame(parent)
    ,_d(new ConfigurationDiaplayerPrivate(this))
{
    setupUI();
}

ConfigurationDiaplayer::ConfigurationDiaplayer(QList<SchemaConfiguration> configs, QWidget* parent)
    :QFrame(parent)
    ,_d(new ConfigurationDiaplayerPrivate(this))
{
    _d->mConfigs= configs;
    setupUI();
}

ConfigurationDiaplayer::~ConfigurationDiaplayer()
{
    _d->mConfigs.clear();
}

void ConfigurationDiaplayer::reloadConfigurations(QList<SchemaConfiguration> configs)
{
    pTitleViewModel->clear();
    pConfigView->clear();

    _d->mConfigs = configs;

    fillView();
}

void ConfigurationDiaplayer::addConfiguration(SchemaConfiguration config)
{
    addConfiguration(config, nullptr);
}

void ConfigurationDiaplayer::addConfiguration(SchemaConfiguration config, ConfigurationTitleDiaplayer* parent)
{
    ConfigurationTitleModelItem* titleModelItem = new ConfigurationTitleModelItem(config.title);
    if(parent)
    {
        QStandardItem* parentItem = parent->getModelItem();
        parentItem->appendRow(titleModelItem);
    }
    else
    {
        pTitleViewModel->appendRow(titleModelItem);
    }

    ConfigurationTitleDiaplayer* titleDiaplyer = new ConfigurationTitleDiaplayer(config.title, _d, parent, nullptr);
    titleDiaplyer->setModelItem(titleModelItem);

    titleModelItem->setTitleDisplayer(titleDiaplyer);

    pConfigView->addCustomListItemWidget(titleDiaplyer);

    // 添加属性
    ConfigurationPropertyDiaplayerCreator* creator = ConfigurationPropertyDiaplayerCreator::instance();
    for(const auto &property : config.properties.values())
    {
        ConfigurationPropertyDiaplayer* propertyWidget = creator->createDiaplayer(property, titleDiaplyer, _d);
        if(propertyWidget)
        {
            QListWidgetItem* propertyItem = propertyWidget->getOrCreateListItem();
            pConfigView->addCustomListItemWidget(propertyWidget);

            // 关联设置项变更的信号
            connect(propertyWidget, &ConfigurationPropertyDiaplayer::propertyValueChanged, this, &ConfigurationDiaplayer::propertyValueChanged);
            connect(propertyWidget, &ConfigurationPropertyDiaplayer::focusIn, this, [this, propertyItem](){
                if(pConfigView->currentItem() != propertyItem)
                {
                    pConfigView->setCurrentItem(propertyItem);
                }
            });
        }
    }


    for(const auto &subConfig : config.configuration)
    {
        addConfiguration(subConfig, titleDiaplyer);
    }
}

void ConfigurationDiaplayer::setSearchAutoResponseInterval(unsigned int interval)
{
    _d->mSearchAutoResponseInterval = interval;
    pSearchInput->updateInterval();
}

void ConfigurationDiaplayer::setSearchFilterButtonHide(bool hide)
{
    QList<QPushButton*> list;

    if(!hide)
    {
        QPushButton* filter = new QPushButton();
        filter->setObjectName("filter");
        filter->setCursor(Qt::ArrowCursor);
        list << filter;
    }

    QPushButton* clean = new QPushButton();
    clean->setObjectName("clean");
    clean->setCursor(Qt::ArrowCursor);
    connect(clean, &QPushButton::clicked, pSearchInput, &QLineEdit::clear);
    connect(clean, &QPushButton::clicked, pSearchInput, &ConfigurationSearchBar::cancelSearch);
    list << clean;

    pSearchInput->clearToolButton();
    pSearchInput->addToolButton(list);
}

void ConfigurationDiaplayer::setSearchHighlightBackgroundColor(const QString& color)
{
    if(QColor::isValidColorName(color))
    {
        _d->mHighlightBackgroundColor = color;
    }
}

void ConfigurationDiaplayer::setSearchHighlightForegroundColor(const QString& color)
{
    if(QColor::isValidColorName(color))
    {
        _d->mHighlightForegroundColor = color;
    }
}

void ConfigurationDiaplayer::setInputWarningText(const QString& warn)
{
    _d->mWarningText = warn;
}

void ConfigurationDiaplayer::setInputWarningTextBackgroundColor(const QString& color)
{
    if(QColor::isValidColorName(color))
    {
        _d->mWarningTextBackgroundColor = color;
    }
}

void ConfigurationDiaplayer::setInputWarningTextForegroundColor(const QString& color)
{
    if(QColor::isValidColorName(color))
    {
        _d->mWarningTextForegroundColor = color;
    }
}

SchemaProperty ConfigurationDiaplayer::propertyFromJson(QJsonObject property, const QString id)
{
    ConfigurationPropertyDiaplayerCreator* ins = ConfigurationPropertyDiaplayerCreator::instance();
    Q_ASSERT(ins->isValid(property));
    return ConfigurationPropertyDiaplayerCreator::instance()->propertyFromJson(property,id);
}

bool ConfigurationDiaplayer::isValidConfigurationFromJson(QJsonObject config)
{
    if(!config.contains("title")
       || (config.contains("title") && !config.value("title").isString())
       || (config.contains("properties") && !config.value("properties").isObject())
       || (config.contains("configuration") && !config.value("configuration").isArray() && !config.value("configuration").isObject()))
    {
        return false;
    }

    if(config.contains("properties"))
    {
        QJsonObject properties = config.value("properties").toObject();
        ConfigurationPropertyDiaplayerCreator* ins = ConfigurationPropertyDiaplayerCreator::instance();
        for(auto key : properties.keys())
        {
            auto value = properties.value(key);
            if(!value.isObject())
            {
                return false;
            }

            if(!ins->isValid(value.toObject()))
            {
                return false;
            }
        }
    }

    if(config.contains("configuration"))
    {
        if(config.value("configuration").isArray())
        {
            for(auto ele : config.value("configuration").toArray())
            {
                if(!ele.isObject())
                {
                    return false;
                }
                if(!isValidConfigurationFromJson(ele.toObject()))
                {
                    return false;
                }
            }
        }
        else if(config.value("configuration").isObject())
        {
            if(!isValidConfigurationFromJson(config.value("configuration").toObject()))
            {
                return false;
            }
        }
    }

    return true;
}

SchemaConfiguration ConfigurationDiaplayer::configurationFromJson(QJsonObject config)
{
    Q_ASSERT(isValidConfigurationFromJson(config));
    QMap<ConfigurationId,SchemaProperty> properties;
    QList<SchemaConfiguration> configuration;

    if(config.contains("properties"))
    {
        QJsonObject propertiesObj = config.value("properties").toObject();
        ConfigurationPropertyDiaplayerCreator* ins = ConfigurationPropertyDiaplayerCreator::instance();
        for(auto key : propertiesObj.keys())
        {
            auto value = propertiesObj.value(key).toObject();
            properties.insert(key, ins->propertyFromJson(value, key));
        }
    }

    if(config.contains("configuration"))
    {
        if(config.value("configuration").isArray())
        {
            configuration = configurationFromJsonArray(config.value("configuration").toArray());
        }
        else if(config.value("configuration").isObject())
        {
            configuration << configurationFromJson(config.value("configuration").toObject());
        }
    }

    return {
        config.value("title").toString(),
        properties,
        configuration
    };
}

QList<SchemaConfiguration> ConfigurationDiaplayer::configurationFromJsonArray(QJsonArray configs)
{
    QList<SchemaConfiguration> arr;
    for (auto config : configs)
    {
        if(isValidConfigurationFromJson(config.toObject()))
        {
            arr << configurationFromJson(config.toObject());
        }
    }
    return arr;
}

void ConfigurationDiaplayer::updateCurrentTitle(QListWidgetItem* item)
{
    if(!_d->sCanUpdateCurrentTitle)
    {
        return;
    }
    QWidget* itemWidget = pConfigView->itemWidget(item);
    if(itemWidget)
    {
        QModelIndex index;
        {
            ConfigurationTitleDiaplayer* titleDisplayer = itemWidget->findChild<ConfigurationTitleDiaplayer*>();
            if(titleDisplayer)
            {
                QStandardItem* item = titleDisplayer->getModelItem();
                index = pTitleViewModel->indexFromItem(item);
            }
        }
        {
            ConfigurationPropertyDiaplayer* propertyDisplayer = itemWidget->findChild<ConfigurationPropertyDiaplayer*>();
            if(propertyDisplayer)
            {
                propertyDisplayer->print();
                QStandardItem* item = propertyDisplayer->title()->getModelItem();
                index = pTitleViewModel->indexFromItem(item);
            }
        }

        if(index.isValid() && pTitleView->currentIndex() != index)
        {
            pTitleView->collapseAll();
            pTitleView->setCurrentIndex(index);
            pTitleView->expand(index);
        }
    }
}

bool ConfigurationDiaplayer::search(const QString& text)
{
    pConfigView->cancelSearch();
    return pConfigView->search(text);
}

void ConfigurationDiaplayer::cancelSearch()
{
    pConfigView->cancelSearch();
}

void ConfigurationDiaplayer::setupUI()
{
    pSearchInput = new ConfigurationSearchBar(_d, this);
    {
        pSearchInput->setObjectName("serachBar");
        pSearchInput->setPlaceholderText("搜索设置");
        pSearchInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        pSearchInput->setFixedHeight(25);

        {
            setSearchFilterButtonHide(true);
        }

        connect(pSearchInput, &ConfigurationSearchBar::editingFinished, this, [this](){
            QString text = pSearchInput->text();
            if(text.isEmpty())
            {
                cancelSearch();
            }
            else
            {
                search(pSearchInput->text());
            }
        });
        connect(pSearchInput, &ConfigurationSearchBar::cancelSearch, this, [this](){
            cancelSearch();
        });
    }

    pTitleViewModel = new QStandardItemModel(pTitleView);
    pTitleView = new QTreeView(this);
    {
        connect(pTitleView, &QTreeView::clicked, this, [this](const QModelIndex &index){
            QStandardItem* item = pTitleViewModel->itemFromIndex(index);
            ConfigurationTitleModelItem* titleItem = dynamic_cast<ConfigurationTitleModelItem*>(item);
            if(titleItem){
                auto displayer = titleItem->getTitleDisplayer();
                QListWidgetItem* ListTitleitem = displayer->getOrCreateListItem();
                auto listTileIndex = pConfigView->indexFromItem(ListTitleitem);

                _d->sCanUpdateCurrentTitle = false;

                pConfigView->scrollTo(listTileIndex, QListWidget::PositionAtTop);
                if(pConfigView->currentIndex() != listTileIndex)
                {
                    pConfigView->setCurrentIndex(listTileIndex);
                }

                _d->sCanUpdateCurrentTitle = true;
            }

            if(pTitleView->isExpanded(index))
            {
                pTitleView->collapse(index);
            }
            else
            {
                pTitleView->expand(index);
            }
        });
        pTitleView->setObjectName("tree");
        pTitleView->setEditTriggers(QTreeView::NoEditTriggers);
        pTitleView->header()->setVisible(false);
        pTitleView->setExpandsOnDoubleClick(false);

        pTitleView->setModel(pTitleViewModel);
    }

    pConfigView = new ConfigurationListWidget(_d, this);
    {
        connect(pConfigView->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value){
            // qDebug() << "valueChanged on item"<< value;
            QWidget* viewport = pConfigView->viewport();
            QPointF globalPos = viewport->mapToGlobal(QPointF(viewport->rect().x() + 1, viewport->rect().y() + 1));
            QListWidgetItem* item = pConfigView->itemAt(pConfigView->mapFromGlobal(globalPos).toPoint());
            if(item)
            {
                // qDebug() << "valid item";
                updateCurrentTitle(item);
            }
        });
        pConfigView->setObjectName("list");
        pConfigView->setSelectionMode(QListWidget::SingleSelection);
        pConfigView->viewport()->installEventFilter(this);
        pConfigView->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    }
    // pTitlePreView = new QListWidget(this);

    fillView();

    QSplitter* hLayout = new QSplitter(this);
    hLayout->addWidget(pTitleView);
    hLayout->addWidget(pConfigView);
    hLayout->setStretchFactor(0,1);
    hLayout->setStretchFactor(1,2);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(10, 10, 10, 10);
    vLayout->setSpacing(10);
    vLayout->addWidget(pSearchInput);
    vLayout->addWidget(hLayout);

    this->setLayout(vLayout);
}

void ConfigurationDiaplayer::fillView()
{
    for(const SchemaConfiguration &config : std::as_const(_d->mConfigs))
    {
        addConfiguration(config);
    }
}

/**************************************************/

ConfigurationSearchBar::ConfigurationSearchBar(ConfigurationDiaplayerPrivate* data, QWidget* parent)
    :QLineEdit(parent)
    ,pData(data)
{
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(pData->mSearchAutoResponseInterval);
    connect(timer, &QTimer::timeout, this, &ConfigurationSearchBar::editingFinished);
    connect(this, &ConfigurationSearchBar::editingFinished, timer, &QTimer::stop);
    connect(this, &ConfigurationSearchBar::textChanged, this, [timer](){
        timer->stop();
        timer->start();
    });

    updateInterval = [this, timer]()->void{
        timer->setInterval(pData->mSearchAutoResponseInterval);
    };
}

void ConfigurationSearchBar::addToolButton(QList<QPushButton*> buttons)
{
    for(auto button : buttons)
    {
        QWidgetAction* buttonAction = new QWidgetAction(this);
        buttonAction->setDefaultWidget(button);
        this->addAction(buttonAction,TrailingPosition);
    }
}

void ConfigurationSearchBar::clearToolButton()
{
    QList<QAction*> actions = this->actions();
    for(auto action : actions)
    {
        this->removeAction(action);
    }
}

/**************************************************/

void ConfigurationListWidget::addCustomListItemWidget(ConfigurationListItemWidget* w)
{
    QWidget* container = createContainer(w);

    QListWidgetItem* item = w->getOrCreateListItem();
    item->setSizeHint(QSize(this->width(),container->height()));

    this->addItem(item);
    this->setItemWidget(item, container);
}

bool ConfigurationListWidget::search(const QString& text)
{
    for(int i = 0;i < this->count(); ++i)
    {
        auto item = this->item(i);
        QWidget* w = this->itemWidget(item);
        if(w)
        {
            ConfigurationListItemWidget* sb = w->findChild<ConfigurationListItemWidget*>();
            if(sb)
            {
                if(!sb->search(text))
                {
                    this->setRowHidden(i, true);
                }
            }
        }
    }
    return true;
}

void ConfigurationListWidget::cancelSearch()
{
    for(int i = 0;i < this->count(); ++i)
    {
        auto item = this->item(i);
        QWidget* w = this->itemWidget(item);
        if(w)
        {
            ConfigurationListItemWidget* sb = w->findChild<ConfigurationListItemWidget*>();
            if(sb)
            {
                sb->cancelSearch();
            }
        }
        this->setRowHidden(i, false);
    }

    this->scrollTo(this->currentIndex(), ConfigurationListWidget::PositionAtCenter);
}

QWidget* ConfigurationListWidget::createContainer(QWidget* content)
{
    content->adjustSize();

    QWidget* container = new QWidget();
    container->setFocusProxy(content);
    container->setObjectName("ItemContainer");

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setSpacing(0);
    layout->setContentsMargins(30, 10, 30, 10);
    layout->addWidget(content);

    container->adjustSize();

    return container;
}

/**************************************************/

ConfigurationListItemWidget::ConfigurationListItemWidget(ConfigurationDiaplayerPrivate* data, QWidget* parent)
    :QFrame(parent)
    ,pData(data)
{

}

QListWidgetItem* ConfigurationListItemWidget::getOrCreateListItem()
{
    if(!pListItem)
    {
        pListItem = new QListWidgetItem;
        // pListItem->setSizeHint(this->size());
    }

    return pListItem;
}

/**************************************************/

ConfigurationTitleDiaplayer::ConfigurationTitleDiaplayer(const QString& title, ConfigurationDiaplayerPrivate* data, ConfigurationTitleDiaplayer* parent, QWidget* parentW)
    :ConfigurationListItemWidget(data, parentW)
    ,mTitle(title)
    ,pParent(parent)
{
    // this->setFixedHeight(35);

    QLabel* lb = new QLabel(this);
    lb->setObjectName("title");
    lb->setProperty("titleLevel",level());
    lb->setFont(QFont("yahei",fontSize()));
    lb->setText(title);


    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(lb);
}

bool ConfigurationTitleDiaplayer::search(const QString& text)
{
    bool has = mTitle.contains(text, Qt::CaseInsensitive);
    if(has)
    {
        QString oText(mTitle);
        int len = text.size();
        int from = 0;
        int index = -1;
        QList<int> indexList;
        while ((index = oText.indexOf(text, from, Qt::CaseInsensitive)) != -1) {
            indexList << index;
            from += (index + len);
        }
        for(auto i = indexList.size() - 1;i >= 0;--i)
        {
            int indexPos = indexList.at(i);
            oText.replace(indexPos, len, this->createHighlightText(oText.sliced(indexPos, len), pData->mHighlightBackgroundColor, pData->mHighlightForegroundColor));
        }

        QLabel* lb = this->findChild<QLabel*>("title");
        lb->setText(oText);
    }
    return has;
}

void ConfigurationTitleDiaplayer::cancelSearch()
{
    QLabel* lb = this->findChild<QLabel*>("title");
    lb->setText(mTitle);
}

void ConfigurationTitleDiaplayer::addChild(ConfigurationTitleDiaplayer* child)
{
    if(!mChildren.contains(child))
    {
        mChildren << child;
    }
}

int ConfigurationTitleDiaplayer::fontSize()
{
    int baseSize = 18;
    ConfigurationTitleDiaplayer * p = pParent;
    while (p) {
        baseSize -= 2;
        p = p->pParent;
    }
    return baseSize;
}

int ConfigurationTitleDiaplayer::level()
{
    int baseLevel = 1;
    ConfigurationTitleDiaplayer * p = pParent;
    while (p) {
        baseLevel++;
        p = p->pParent;
    }
    return baseLevel;
}

/**************************************************/

ConfigurationPropertyDiaplayer::ConfigurationPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data, QWidget* parent)
    :ConfigurationListItemWidget(data, parent)
    ,mProperty(property)
    ,pTitle(title)
{
}

void ConfigurationPropertyDiaplayer::print()
{
    // qDebug() << "id:" << mProperty.id << "title" << pTitle->mTitle;
    // pTitle->print();
}

QLabel* ConfigurationPropertyDiaplayer::createIdLabel()
{
    QLabel* id = new QLabel(this);
    id->setObjectName("id");
    id->setWordWrap(true);
    id->setText(tr(mProperty.id.toStdString().c_str()));
    return id;
}

QLabel* ConfigurationPropertyDiaplayer::createDescriptionLabel()
{
    QLabel* desc = new QLabel(this);
    desc->setObjectName("description");
    desc->setWordWrap(true);
    desc->setText(tr(mProperty.description.toStdString().c_str()));
    desc->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
    return desc;
}

bool ConfigurationPropertyDiaplayer::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::InputMethodQuery)
    {
        emit focusIn();
    }
    else if(event->type() == QEvent::MouseButtonPress)
    {
        emit focusIn();
    }
    else if(event->type() == QEvent::Wheel)
    {
        event->ignore();
        return true;
    }
    // qDebug() << event->type();
    return ConfigurationListItemWidget::eventFilter(watched, event);
}

/**************************************************/

StringPropertyDiaplayer::StringPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data, QWidget* parent)
    :ConfigurationPropertyDiaplayer(property,title,data,parent)
{
    QLabel* id = this->createIdLabel();
    QLabel* desc = this->createDescriptionLabel();

    QWidget* input = nullptr;
    if(!property.enumValue.isEmpty())
    {
        QComboBox* cb= new QComboBox();
        cb->setFixedWidth(320);
        for(int i = 0; i < property.enumValue.size(); ++i)
        {
            QVariant ele = property.enumValue.at(i);
            QString name = ele.toString();
            if(name.isEmpty())
            {
                continue;
            }
            if(i < property.enumItems.size())
            {
                QString desc = property.enumItems.at(i).description;
                if(!desc.isEmpty())
                {
                    name += " ";
                    name += desc;
                }
            }
            cb->addItem(name);
        }

        cb->setCurrentText(mProperty.defaultValue.toString());
        connect(cb, &QComboBox::currentTextChanged, this, [this](const QString &text){
            QString lastValue = mProperty.value.toString();
            lastValue = lastValue.isEmpty() ? mProperty.defaultValue.toString() : lastValue;
            mProperty.value = text;
            emit propertyValueChanged(mProperty, lastValue, mProperty.value);
        });

        input = cb;
    }
    else
    {
        QLineEdit* lineInput = new QLineEdit(this);
        lineInput->setFixedHeight(25);
        lineInput->setText(mProperty.defaultValue.toString());
        connect(lineInput, &QLineEdit::editingFinished, this, [this, lineInput](){
            QString lastValue = mProperty.value.toString();
            lastValue = lastValue.isEmpty() ? mProperty.defaultValue.toString() : lastValue;
            mProperty.value = lineInput->text();
            emit propertyValueChanged(mProperty, lastValue, mProperty.value);
        });

        input = lineInput;
    }
    input->setObjectName("input");
    input->installEventFilter(this);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(10);
    vLayout->addWidget(id);
    vLayout->addWidget(desc);
    vLayout->addWidget(input);

    this->setFocusProxy(input);
}

bool StringPropertyDiaplayer::search(const QString& text)
{
    bool hasId = mProperty.id.contains(text, Qt::CaseInsensitive);
    bool hasDesc = mProperty.description.contains(text, Qt::CaseInsensitive);

    if(hasId)
    {
        QLabel* id = this->findChild<QLabel*>("id");
        id->setText(this->highlightText(mProperty.id, text, pData->mHighlightBackgroundColor, pData->mHighlightForegroundColor));
    }

    if(hasDesc)
    {
        QLabel* desc = this->findChild<QLabel*>("description");
        desc->setText(this->highlightText(mProperty.description, text, pData->mHighlightBackgroundColor, pData->mHighlightForegroundColor));
    }

    return hasId || hasDesc;
}

void StringPropertyDiaplayer::cancelSearch()
{
    QLabel* id = this->findChild<QLabel*>("id");
    id->setText(mProperty.id);
    QLabel* desc = this->findChild<QLabel*>("description");
    desc->setText(mProperty.description);
}

/**************************************************/

NumberPropertyDiaplayer::NumberPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data, QWidget* parent)
    :ConfigurationPropertyDiaplayer(property,title,data,parent)
{
    QLabel* id = this->createIdLabel();
    QLabel* desc = this->createDescriptionLabel();

    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    {
        QString rowText = desc->text();

        QTimer* timer = new QTimer(group);
        {
            timer->setSingleShot(true);
            connect(timer, &QTimer::timeout, this, [group, desc, rowText](){
                if(group->state() == QSequentialAnimationGroup::Stopped)
                {
                    desc->setText(rowText);
                }
            });
        }

        connect(group, &QSequentialAnimationGroup::finished, [group, timer](){
            timer->stop();
            timer->start(2000);

            for(int i = 0;i < group->animationCount(); ++i)
            {
                auto animation = group->animationAt(i);
                animation->deleteLater();
            }

            group->clear();
        });
    }


    QLineEdit* input = new QLineEdit(this);
    {
        input->setObjectName("input");
        input->installEventFilter(this);
        input->setFixedHeight(25);
        input->setValidator(new QDoubleValidator(-99999999.9999, 99999999.9999, 4));
        double v = mProperty.defaultValue.toDouble();
        auto str = QString::number(v);
        input->setText(str);

        connect(input, &QLineEdit::editingFinished, this, [this, input](){
            bool ok;
            double oldValue = mProperty.value.toDouble(&ok);
            if(!ok)
            {
                oldValue = mProperty.defaultValue.toDouble(&ok);
                Q_ASSERT(ok);
            }

            mProperty.value = input->text().toDouble(&ok);
            Q_ASSERT(ok);
            emit propertyValueChanged(mProperty, oldValue, mProperty.value);
        });
        connect(input, &QLineEdit::inputRejected, this, [this, desc, group](){
            if(group->state() == QAbstractAnimation::Stopped)
            {
                group->clear();;

                QPointF rowPos = desc->pos();
                QPointF nowPos = rowPos;
                QPropertyAnimation * animation = new QPropertyAnimation(desc, "pos", this);
                animation->setDuration(50);
                animation->setStartValue(rowPos);
                animation->setEndValue(nowPos = QPointF(nowPos.x() + 50, nowPos.y()));

                QPropertyAnimation * animation2 = new QPropertyAnimation(desc, "pos", this);
                animation2->setDuration(50);
                animation2->setStartValue(nowPos);
                animation2->setEndValue(nowPos = QPointF(nowPos.x() - 50, nowPos.y()));

                QPropertyAnimation * animation3 = new QPropertyAnimation(desc, "pos", this);
                animation3->setDuration(100);
                animation3->setStartValue(nowPos);
                animation3->setEndValue(nowPos = QPointF(nowPos.x() + 50, nowPos.y()));

                QPropertyAnimation * animation4 = new QPropertyAnimation(desc, "pos", this);
                animation4->setDuration(50);
                animation4->setStartValue(nowPos);
                animation4->setEndValue(nowPos = QPointF(nowPos.x() - 50, nowPos.y()));

                group->addAnimation(animation);
                // group->addPause(100);
                group->addAnimation(animation2);
                group->addAnimation(animation3);
                group->addAnimation(animation4);

                auto nt = QString("<font style=\"background-color:%1; color:%2;\">%3</font>").arg(pData->mWarningTextBackgroundColor, pData->mWarningTextForegroundColor, pData->mWarningText);
                desc->setText(nt);
                group->start();
            }
        });
    }


    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(10);
    vLayout->addWidget(id);
    vLayout->addWidget(desc);
    vLayout->addWidget(input);

    this->setFocusProxy(input);
}

bool NumberPropertyDiaplayer::search(const QString& text)
{
    bool hasId = mProperty.id.contains(text, Qt::CaseInsensitive);
    bool hasDesc = mProperty.description.contains(text, Qt::CaseInsensitive);

    if(hasId)
    {
        QLabel* id = this->findChild<QLabel*>("id");
        id->setText(this->highlightText(mProperty.id, text, pData->mHighlightBackgroundColor, pData->mHighlightForegroundColor));
    }

    if(hasDesc)
    {
        QLabel* desc = this->findChild<QLabel*>("description");
        desc->setText(this->highlightText(mProperty.description, text, pData->mHighlightBackgroundColor, pData->mHighlightForegroundColor));
    }

    return hasId || hasDesc;
}

void NumberPropertyDiaplayer::cancelSearch()
{
    QLabel* id = this->findChild<QLabel*>("id");
    id->setText(mProperty.id);
    QLabel* desc = this->findChild<QLabel*>("description");
    desc->setText(mProperty.description);
}

/**************************************************/

BooleanPropertyDiaplayer::BooleanPropertyDiaplayer(SchemaProperty property, ConfigurationTitleDiaplayer* title, ConfigurationDiaplayerPrivate* data, QWidget* parent)
    :ConfigurationPropertyDiaplayer(property,title,data,parent)
{
    QLabel* id = this->createIdLabel();
    QCheckBox* check = new QCheckBox(this);
    check->setObjectName("description");
    check->installEventFilter(this);
    check->setChecked(mProperty.defaultValue.toBool());
    connect(check, &QCheckBox::stateChanged, this, [this](int state){
        bool isChecked = state == Qt::Checked ? true : false;
        bool oldValue = mProperty.value.toBool() || mProperty.defaultValue.toBool();

        mProperty.value = isChecked;
        emit propertyValueChanged(mProperty, oldValue, isChecked);
    });
    check->setChecked(mProperty.defaultValue.toBool());
    QLabel* desc = this->createDescriptionLabel();
    desc->installEventFilter(this);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(10);
    hLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hLayout->addWidget(check);
    hLayout->addWidget(desc);


    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(10);
    vLayout->addWidget(id);
    vLayout->addLayout(hLayout);

    this->setFocusProxy(check);
}

bool BooleanPropertyDiaplayer::search(const QString& text)
{
    bool hasId = mProperty.id.contains(text, Qt::CaseInsensitive);
    bool hasDesc = mProperty.description.contains(text, Qt::CaseInsensitive);

    if(hasId)
    {
        QLabel* id = this->findChild<QLabel*>("id");
        id->setText(this->highlightText(mProperty.id, text, pData->mHighlightBackgroundColor, pData->mHighlightForegroundColor));
    }

    if(hasDesc)
    {
        QLabel* desc = this->findChild<QLabel*>("description");
        desc->setText(this->highlightText(mProperty.description, text, pData->mHighlightBackgroundColor, pData->mHighlightForegroundColor));
    }

    return hasId || hasDesc;
}

void BooleanPropertyDiaplayer::cancelSearch()
{
    QLabel* id = this->findChild<QLabel*>("id");
    id->setText(mProperty.id);
    QLabel* desc = this->findChild<QLabel*>("description");
    desc->setText(mProperty.description);
}

bool BooleanPropertyDiaplayer::eventFilter(QObject* watched, QEvent* event)
{
    if(watched->inherits("QLabel") && watched->objectName() == "description" && event->type() == QEvent::MouseButtonPress)
    {
         QCheckBox* desc = this->findChild<QCheckBox*>("description");
         desc->setChecked(!desc->isChecked());
    }
    return ConfigurationPropertyDiaplayer::eventFilter(watched, event);
}

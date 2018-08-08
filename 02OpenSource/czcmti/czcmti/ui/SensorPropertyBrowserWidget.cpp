#include "SensorPropertyBrowserWidget.h"
#include <functional>
#include "bll/sensorbll.h"

SensorPropertyBrowserWidget::SensorPropertyBrowserWidget(QWidget *parent) :
    QtTreePropertyBrowser(parent)
{
    QtBoolPropertyManager *boolManager = new QtBoolPropertyManager(parent);
    connect(boolManager, &QtBoolPropertyManager::valueChanged, this, &SensorPropertyBrowserWidget::boolManager_valueChanged);
    m_intPropManager = new QtIntPropertyManager(parent);
    QtStringPropertyManager *stringManager = new QtStringPropertyManager(parent);
    QtSizePropertyManager *sizeManager = new QtSizePropertyManager(parent);
    QtRectPropertyManager *rectManager = new QtRectPropertyManager(parent);
    QtSizePolicyPropertyManager *sizePolicyManager = new QtSizePolicyPropertyManager(parent);
    QtEnumPropertyManager *enumManager = new QtEnumPropertyManager(parent);
    QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(parent);

    QtProperty *m_voltageProperty = groupManager->addProperty("Voltage");
    m_voltDovdd = m_intPropManager->addProperty("DOVDD");
    m_voltageProperty->addSubProperty(m_voltDovdd);
    m_intPropManager->setValue(m_voltDovdd, 0);

    m_voltDvdd = m_intPropManager->addProperty("DVDD");
    m_voltageProperty->addSubProperty(m_voltDvdd);
    m_intPropManager->setValue(m_voltDvdd, 0);

    m_voltAvdd = m_intPropManager->addProperty("AVDD");
    m_voltageProperty->addSubProperty(m_voltAvdd);
    m_intPropManager->setValue(m_voltAvdd, 0);

    m_voltAfvcc = m_intPropManager->addProperty("AFVCC");
    m_voltageProperty->addSubProperty(m_voltAfvcc);
    m_intPropManager->setValue(m_voltAfvcc, 0);

    m_voltVpp = m_intPropManager->addProperty("VPP");
    m_voltageProperty->addSubProperty(m_voltVpp);
    m_intPropManager->setValue(m_voltVpp, 0);


    QtProperty *propBaseInfo = groupManager->addProperty("Base Info");

    QtProperty *propMclk = m_intPropManager->addProperty("MCLK(MHz)");
    propBaseInfo->addSubProperty(propMclk);
    m_intPropManager->setValue(propMclk, 24);

    QtProperty *propSpeed = enumManager->addProperty("Speed");
    propBaseInfo->addSubProperty(propSpeed);
    QStringList enumSpeedNames;
    enumSpeedNames << "100k" << "400k";
    enumManager->setEnumNames(propSpeed, enumSpeedNames);
    enumManager->setValue(propSpeed, 1);

    QtProperty *item3 = rectManager->addProperty("geometry");
    propBaseInfo->addSubProperty(item3);

    QtProperty *item4 = sizePolicyManager->addProperty("sizePolicy");
    propBaseInfo->addSubProperty(item4);

    QtProperty *item5 = sizeManager->addProperty("sizeIncrement");
    propBaseInfo->addSubProperty(item5);

//    QtProperty *item8 = enumManager->addProperty("direction");
//    QStringList enumNames;
//    enumNames << "Up" << "Right" << "Down" << "Left";
//    enumManager->setEnumNames(item8, enumNames);
//    QMap<int, QIcon> enumIcons;
//    enumIcons[0] = QIcon(":/demo/images/up.png");
//    enumIcons[1] = QIcon(":/demo/images/right.png");
//    enumIcons[2] = QIcon(":/demo/images/down.png");
//    enumIcons[3] = QIcon(":/demo/images/left.png");
//    enumManager->setEnumIcons(item8, enumIcons);
//    propBaseInfo->addSubProperty(item8);

    QtProperty *item9 = m_intPropManager->addProperty("value");
    m_intPropManager->setRange(item9, -100, 100);
    propBaseInfo->addSubProperty(item9);

    QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(parent);
    QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(parent);
    //QtSliderFactory *sliderFactory = new QtSliderFactory(parent);
    //QtScrollBarFactory *scrollBarFactory = new QtScrollBarFactory(parent);
    QtLineEditFactory *lineEditFactory = new QtLineEditFactory(parent);
    QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(parent);

    setFactoryForManager(boolManager, checkBoxFactory);
    setFactoryForManager(m_intPropManager, spinBoxFactory);
    setFactoryForManager(stringManager, lineEditFactory);
    setFactoryForManager(sizeManager->subIntPropertyManager(), spinBoxFactory);
    setFactoryForManager(rectManager->subIntPropertyManager(), spinBoxFactory);
    setFactoryForManager(sizePolicyManager->subIntPropertyManager(), spinBoxFactory);
    setFactoryForManager(sizePolicyManager->subEnumPropertyManager(), comboBoxFactory);
    setFactoryForManager(enumManager, comboBoxFactory);
    addProperty(propBaseInfo);
    addProperty(m_voltageProperty);
}

SensorPropertyBrowserWidget::~SensorPropertyBrowserWidget()
{
}

void SensorPropertyBrowserWidget::SetSensor(const Sensor &sensor, bool isDefault)
{
    m_sensor = sensor;
//    clearProperties();
//    addProperties();

    QString strVoltage = isDefault ? m_sensor.VoltageDef : m_sensor.Voltage;
    std::vector<T_Power> powers;
    if (SensorBLL::ParseVoltageList(strVoltage, powers)) {
        for (uint i = 0; i < powers.size(); i++) {
            switch (powers[i].Id) {
            case PI_DVDD:
                m_intPropManager->setValue(m_voltDvdd, powers[i].Value);
                break;
            case PI_AVDD:
                m_intPropManager->setValue(m_voltAvdd, powers[i].Value);
                break;
            case PI_DOVDD:
                m_intPropManager->setValue(m_voltDovdd, powers[i].Value);
                break;
            case PI_AFVCC:
                m_intPropManager->setValue(m_voltAfvcc, powers[i].Value);
                break;
            case PI_VPP:
                m_intPropManager->setValue(m_voltVpp, powers[i].Value);
                break;
            default:
                break;
            }
        }
    }
}

void SensorPropertyBrowserWidget::GetSensor(Sensor &sensor)
{
    sensor = m_sensor;
}

void SensorPropertyBrowserWidget::ShowAdvanced(bool show)
{
    (void)show;
}

void SensorPropertyBrowserWidget::boolManager_valueChanged(QtProperty *property, bool val)
{
    if (!m_mapProperty2Name.contains(property))
        return;

    QString name = m_mapProperty2Name[property];
    if (name == QLatin1String("PclkPol")) {
        m_sensor.PclkPol = val ? 1 : 0;
    } else if (name == QLatin1String("DataPol")) {
        m_sensor.DataPol = val ? 1 : 0;
    } else if (name == QLatin1String("HsyncPol")) {
        m_sensor.HsyncPol = val ? 1 : 0;
    } else if (name == QLatin1String("VsyncPol")) {
        m_sensor.VsyncPol = val ? 1 : 0;
    } else if (name == QLatin1String("Pwdn")) {
        m_sensor.Pwdn = val ? 1 : 0;
    } else if (name == QLatin1String("Reset")) {
        m_sensor.Reset = val ? 1 : 0;
    }
}

void SensorPropertyBrowserWidget::clearProperties()
{
//    std::function<void (QtProperty *property)> pfDelAllSubProperties = [&pfDeleteAllSubProperties](QtProperty *property) {
//        QList<QtProperty *> subProperties = property->subProperties();
//        for (int i = 0; i < subProperties.size(); i++) {
//            if (subProperties[i]->subProperties().size() > 0)
//                pfDelAllSubProperties(subProperties[i]);
//            else
//                property->removeSubProperty(subProperties[i]);
//        }
//    };

//    QList<QtProperty *> properties = this->properties();
//    for (int i = 0; i < properties.size(); i++) {
//        pfDelAllSubProperties(properties[i]);
//    }


//    QListIterator<QtProperty *> it(this->properties());
//    while (it.hasNext()) {
//        this->removeProperty(it.next());
//    }
//    d_ptr->m_topLevelProperties.clear();
    this->clear();
}

void SensorPropertyBrowserWidget::addProperties()
{
    //
}

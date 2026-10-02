#include "tokenparameter.h"
#include <QDebug>
#include <QApplication>
#include <QGSettings>
#include <QFontDatabase>
#include <QIcon>
#include <QColor>

using namespace UKUIQQC2Style;
TokenParameter::TokenParameter()
{  
    //UKUIGlobalDTConfig::GlobalDTConfig dtConfig;

}

TokenParameter::~TokenParameter()
{
    deleteInstance();
}

UKUIGlobalDTConfig::GlobalDTConfig* TokenParameter:: getInstance()
{
    if(dt == nullptr)
        dt = new UKUIGlobalDTConfig::GlobalDTConfig();

    return dt;
}

void TokenParameter::deleteInstance()
{
    if(dt){
        delete dt;
        dt = nullptr;
    }
}

//const std::shared_ptr<UKUIQQC2Style::UKUIButton> &TokenParameter::button() const
//{
//    return m_button;
//}

//void TokenParameter::setButton(std::shared_ptr<UKUIQQC2Style::UKUIButton> &newButton)
//{
//    if (m_button == newButton)
//        return;
//    m_button = newButton;
//    emit buttonChanged();
//}

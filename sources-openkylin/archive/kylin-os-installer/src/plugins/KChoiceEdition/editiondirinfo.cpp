/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "editiondirinfo.h"

editionDirInfo::editionDirInfo()
{
    m_EditionButton = new PushButtonCheck(m_edition);
    m_EditionButton->setObjectName("EditionButton");
    m_EditionButton->setStyleSheet("QPushButton{\
                                                   width: 320px;\
                                                   height: 48px;\
                                                   opacity: 1;\
                                                   border-radius: 6px;\
                                                   border:2px;\
                                                   font-size: 14px;\
                                                   font-family: Noto Sans CJK SC;\
                                                   font-weight: 500;\
                                                   text-align:left;\
                                                   line-height: 35px;\
                                                   color: #FFFFFF;\
                                                   background-color: rgba(0, 0, 0, 0.1);\
                                                }"
                                     "QPushButton:hover{background: rgba(0, 0, 0, 0.35)}"
                                     "QPushButton:focus{border: 2px solid #5BADFF;}"
                                     "QPushButton:checked{background: rgba(255, 255, 255, 0.1)}"
//                                   "QPushButton:pressed{background: #119A50}"
                            );

    connect(m_EditionButton,&QPushButton::clicked,this,[=](){emit checkButton_sig(this);});
}

void editionDirInfo::translater(QString langue){
    if(m_EditionButton){
        QString strValue = m_LangtoNameMap[langue];
        if(strValue.isEmpty()){
            strValue = m_edition;
        }
        m_EditionButton->setText( "     " + strValue );
        m_EditionButton->update();
    }
 }

void editionDirInfo::other_clicked_slot(editionDirInfo*  objname)
{
    if(objname){
        if(this == objname){
//            m_EditionButton->setStyleSheet("QPushButton#EditionButton{background-color: rgba(255, 255, 255, 0.1); }" );
            setChecked(true);
            m_EditionButton->setLabelShow(true);
        }else{
//            m_EditionButton->setStyleSheet("QPushButton#EditionButton{background-color: rgba(0, 0, 0, 0.1); }" );
            setChecked(false);
            m_EditionButton->setLabelShow(false);
        }
    }
}

void editionDirInfo::setChecked(bool isSet)
{
    if(isSet){
        m_EditionButton->setStyleSheet("QPushButton{\
                                                       width: 320px;\
                                                       height: 48px;\
                                                       opacity: 1;\
                                                       border-radius: 6px;\
                                                       border:2px;\
                                                       font-size: 14px;\
                                                       font-family: Noto Sans CJK SC;\
                                                       font-weight: 500;\
                                                       text-align:left;\
                                                       line-height: 35px;\
                                                       color: #FFFFFF;\
                                                       background-color: rgba(255, 255, 255, 0.1);\
                                                    }"
                                         "QPushButton:hover{background: rgba(255, 255, 255, 0.35)}"
                                         "QPushButton:focus{border:2px solid #5BADFF;}"
                                         "QPushButton:checked{background: rgba(255, 255, 255, 0.1)}"
    //                                   "QPushButton:pressed{background: #119A50}"
                                );
    }else{
        m_EditionButton->setStyleSheet("QPushButton{\
                                               width: 320px;\
                                               height: 48px;\
                                               opacity: 1;\
                                               border-radius: 6px;\
                                               border:2px;\
                                               font-size: 14px;\
                                               font-family: Noto Sans CJK SC;\
                                               font-weight: 500;\
                                               text-align:left;\
                                               line-height: 35px;\
                                               color: #FFFFFF;\
                                               background-color: rgba(0, 0, 0, 0.1);\
                                            }"
                                 "QPushButton:hover{background: rgba(255, 255, 255, 0.35)}"
                                 "QPushButton:focus{border:2px solid #5BADFF;}"
                                 "QPushButton:checked{background: rgba(255, 255, 255, 0.1)}"
//                                   "QPushButton:pressed{background: #119A50}"
                        );
    }
}

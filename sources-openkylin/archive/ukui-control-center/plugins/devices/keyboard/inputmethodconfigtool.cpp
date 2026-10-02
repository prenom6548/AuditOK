/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "inputmethodconfigtool.h"

#include <QApplication>

InputMethodConfigTool::InputMethodConfigTool(QWidget *parent)
    : QWidget(parent)
{
  init();
  connectDbusSignals();
  connectImModelSignals();
}

void InputMethodConfigTool::init()
{
  if(!m_dbus)
  {
    m_dbus = new FcitxDbusProvider(this);
  }
  if(!isFcitxServiceExists())
  {
    qCritical() << "InputMethodConfigTool(), FcitxService not Exists, will not initialize all data models and views!";
    return;
  }

  m_imModel.reset(new InputMethodListModel(m_dbus, this));
  m_searchDataModel.reset(new SearchDataModel(this));
  m_searchImModel.reset(new SearchInputMethodModel(m_dbus, this));
  m_configManager.reset(new ConfigManager(m_dbus, this));
  m_menuManager.reset(new MenuManager(this));
  m_layout.reset(new LayoutManager(m_dbus, this));
  m_shortcutController.reset(new ShortcutController(m_dbus, this));
}

void InputMethodConfigTool::destroy()
{
  qDebug() << "InputMethodConfigTool func: " << __FUNCTION__ << " line: " << __LINE__;
  m_view.reset();
  m_imModel.reset();
  m_searchDataModel.reset();
  m_searchImModel.reset();
  m_configManager.reset();
  m_menuManager.reset();
  m_layout.reset();
}

void InputMethodConfigTool::connectDbusSignals()
{
  QObject::connect(m_dbus, &FcitxDbusProvider::fcitxDbusServiceEnabled, this,
                   [this]()
                   {
                     init();
                     connectImModelSignals();
                     emit fcitxServiceStatusChanged(true);
                   });
  QObject::connect(m_dbus, &FcitxDbusProvider::fcitxDbusServiceDisabled, this,
                   [this]()
                   {
                     destroy();
                     emit fcitxServiceStatusChanged(false);
                   });
}

void InputMethodConfigTool::connectImModelSignals()
{
  QObject::connect(m_imModel.get(),
                    &InputMethodListModel::availableInputMethodsChanged,
                    m_searchDataModel.get(),
                    &SearchDataModel::onAvailableInputMethodsChanged);
  QObject::connect(m_imModel.get(), &InputMethodListModel::inputMethodsChanged, this,
                   [this](const QString& firstImName)
                   {
                        if (firstImName != m_firstInputMethodName) {
                            m_firstInputMethodName = firstImName;
                            emit firstInputMethodChanged(firstImName);
                        }
                   });
}

void InputMethodConfigTool::connectViewSignals()
{
  QObject::connect(m_view.get(), &QWindow::visibilityChanged, [&](QWindow::Visibility visibility) {
      if (visibility == QWindow::Hidden) {
          if(m_view){
              qDebug() << "InputMethodConfigTool func:" << __FUNCTION__ << " line : " << __LINE__ << ", view hidden, will delete it later";
              m_view->setTransientParent(nullptr);
              m_view->deleteLater();
              m_view.release();
          }
      }
  });
}

void InputMethodConfigTool::showWindow()
{
  qDebug() << "InputMethodConfigTool func :" << __FUNCTION__ << " line : " << __LINE__ ;

  // 控制面板处于显示状态，且配置工具之前弹出过，
  // 那么控制面板在第二次收到组件消息后，如果判断存在模态窗口，则不透传事件消息，
  // 这会导致配置工具隐藏后无法再次显示（配置工具在显示时会设置为模态，在关闭时，由于quickview的机制，是隐藏而不是真正的关闭），
  // 如果配置工具在隐藏时设置为NoModal，则会使控制面板冻结，所以改成了配置工具在隐藏时直接销毁，在销毁后释放uniqueptr的所有权，
  // 后续再显示窗口时直接重新初始化。
  if(!m_view){
      m_view.reset(new InputMethodConfigToolView(
                       m_imModel.get(), m_menuManager.get(), m_configManager.get(),
                       m_layout.get(), m_searchDataModel.get(), m_searchImModel.get(), m_shortcutController.get()));
      connectViewSignals();
  }

  makeParentWindowModality();

  m_view->show();
}

void InputMethodConfigTool::makeParentWindowModality()
{
  QList<QWindow *> windowList = QApplication::topLevelWindows();
  for (auto *window : windowList)
  {
    if (window->title() == "设置")
    {
      m_view->setModality(Qt::WindowModal);
      m_view->setTransientParent(window);
      break;
    }
  }
}

const QString InputMethodConfigTool::currentInputMethodName()
{
  return tr("pinyin && sogou input method");
}

bool InputMethodConfigTool::isFcitxServiceExists()
{
  if (m_dbus == nullptr)
  {
    qWarning() << "isFcitxServiceExists()" << ", dbus is null";
    return false;
  }
  if (!m_dbus->dbusProxy())
  {
    qWarning() << "isFcitxServiceExists()" << ", dbusProxy is null";
    return false;
  }
  if (!m_dbus->dbusProxy()->isValid())
  {
    qWarning() << "isFcitxServiceExists()" << ", dbusProxy is invalid";
    return false;
  }
  return true;
}

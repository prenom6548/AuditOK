/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KMRE_PLATFORM_H_
#define KMRE_PLATFORM_H_

#include <QMap>
#include <QMutex>
#include <memory>
#include "eventdata.h"

namespace kmre {
namespace input {
class Device;
class Manager;
}  // namespace input
}

class KmrePlatform : public QObject {
    Q_OBJECT

 public:
  KmrePlatform(const std::shared_ptr<kmre::input::Manager> &input_manager,
              const uint32_t width, 
              const uint32_t height,
              QObject *parent = 0);
  ~KmrePlatform();


public slots:
  void onSendEventData(QList<EventData> events, bool isTouch, int winId, int eventType);

private:
  std::shared_ptr<kmre::input::Manager> input_manager_;
  std::shared_ptr<kmre::input::Device> screen_;
  std::shared_ptr<kmre::input::Device> mouse_;
  std::shared_ptr<kmre::input::Device> keyboard_;
  QMutex mutex;
};

#endif // KMRE_PLATFORM_H_

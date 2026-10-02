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

#include "validatestate.h"
#include <QSharedPointer>
namespace KInstaller {

Validator::Validator(ValidateStateId state):m_state(state)
{

}

Validator::Validator(ValidateStateId state, const Partition::Ptr& partition)
    :m_partition(partition), m_state(state)
{

}

ValidateStateId Validator::state() const
{
    return m_state;
}

const Partition::Ptr Validator::partition() const
{
    return m_partition;
}

bool Validator::equals(const Validator &validator) const
{
    if(state() != validator.state()) {
        return  false;
    }
    if(partition().isNull() && validator.partition().isNull()) {
        return true;
    } else if(!partition().isNull() && !validator.partition().isNull()) {
        return partition()->m_partitionPath == validator.partition()->m_partitionPath
                && partition()->m_devPath == validator.partition()->m_devPath;
    } else {
        return  false;
    }

}

ValidateState::ValidateState()
    :Validator::Ptr(nullptr)
{

}

ValidateState::ValidateState(Validator* p)
    :Validator::Ptr(p)
{

}

ValidateState::ValidateState(ValidateStateId state)
    :Validator::Ptr(new Validator(state))
{

}

ValidateState::ValidateState(ValidateStateId state, const Partition::Ptr& partition)
    :Validator::Ptr(new Validator(state, partition))
{

}

bool ValidateState::eqauls(const ValidateState &state) const
{
    if(isNull() || state.isNull()) {
        return false;
    }
    return false;//state->equals(*get());
}

ValidateState::operator ValidateStateId() const
{
    return false;//isNull() ? static_cast<ValidateStateId>(InvalidId):get()->state();
}

}

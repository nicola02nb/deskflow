/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2015-2016 Symless Ltd.
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "arch/Arch.h"
#include "ipc/IpcMessage.h"
#include "ipc/IpcServer.h"

#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Invoke;

class IEventQueue;

class MockIpcServer : public IpcServer
{
public:
  MockIpcServer() : m_sendCond(ARCH->newCondVar()), m_sendMutex(ARCH->newMutex())
  {
  }

  ~MockIpcServer()
  {
    if (m_sendCond != NULL) {
      ARCH->closeCondVar(m_sendCond);
    }

    if (m_sendMutex != NULL) {
      ARCH->closeMutex(m_sendMutex);
    }
  }

  MOCK_METHOD(void, listen, (), (override));
  MOCK_METHOD(void, send, (const IpcMessage &, IpcClientType), (override));
  MOCK_METHOD(bool, hasClients, (IpcClientType), (const, override));

  void delegateToFake()
  {
    ON_CALL(*this, send(_, _)).WillByDefault(Invoke(this, &MockIpcServer::mockSend));
  }

  void waitForSend()
  {
    ARCH->waitCondVar(m_sendCond, m_sendMutex, 5);
  }

private:
  void mockSend(const IpcMessage &, IpcClientType)
  {
    ArchMutexLock lock(m_sendMutex);
    ARCH->broadcastCondVar(m_sendCond);
  }

  ArchCond m_sendCond;
  ArchMutex m_sendMutex;
};

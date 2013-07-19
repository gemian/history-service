/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HISTORYDAEMON_H
#define HISTORYDAEMON_H

#include <QObject>
#include <QSharedPointer>
#include "types.h"
#include "textchannelobserver.h"
#include "callchannelobserver.h"

class HistoryWriter;

class HistoryDaemon : public QObject
{
    Q_OBJECT
public:
    HistoryDaemon(QObject *parent = 0);
    ~HistoryDaemon();

private Q_SLOTS:
    void onObserverCreated();
    void onCallEnded(const Tp::CallChannelPtr &channel);
    void onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message);
    void onMessageRead(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken);

private:
    CallChannelObserver mCallObserver;
    TextChannelObserver mTextObserver;
};

#endif

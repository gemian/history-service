/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#include "historydaemon.h"
#include "historyservicedbus.h"
#include "historyserviceadaptor.h"
#include "types.h"

Q_DECLARE_METATYPE(QList< QVariantMap >)

HistoryServiceDBus::HistoryServiceDBus(QObject *parent) :
    QObject(parent), mAdaptor(0), mSignalsTimer(-1)
{
    qDBusRegisterMetaType<QList<QVariantMap> >();
}

bool HistoryServiceDBus::connectToBus()
{
    if (!mAdaptor) {
        mAdaptor = new HistoryServiceAdaptor(this);
    }

    if (!QDBusConnection::sessionBus().registerObject(History::DBusObjectPath, this)) {
        return false;
    }

    return QDBusConnection::sessionBus().registerService(History::DBusService);
}

void HistoryServiceDBus::notifyThreadsAdded(const QList<QVariantMap> &threads)
{
    qDebug() << __PRETTY_FUNCTION__ << threads.count();
    mThreadsAdded << threads;
    triggerSignals();
}

void HistoryServiceDBus::notifyThreadsModified(const QList<QVariantMap> &threads)
{
    qDebug() << __PRETTY_FUNCTION__ << threads.count();
    mThreadsModified << threads;
    triggerSignals();
}

void HistoryServiceDBus::notifyThreadsRemoved(const QList<QVariantMap> &threads)
{
    qDebug() << __PRETTY_FUNCTION__ << threads.count();
    mThreadsRemoved << threads;
    triggerSignals();
}

void HistoryServiceDBus::notifyEventsAdded(const QList<QVariantMap> &events)
{
    qDebug() << __PRETTY_FUNCTION__ << events.count();
    mEventsAdded << events;
    triggerSignals();
}

void HistoryServiceDBus::notifyEventsModified(const QList<QVariantMap> &events)
{
    qDebug() << __PRETTY_FUNCTION__ << events.count();
    mEventsModified << events;
    triggerSignals();
}

void HistoryServiceDBus::notifyEventsRemoved(const QList<QVariantMap> &events)
{
    qDebug() << __PRETTY_FUNCTION__ << events.count();
    mEventsRemoved << events;
    triggerSignals();
}

void HistoryServiceDBus::notifyThreadParticipantsChanged(const QVariantMap &thread,
                                                   const QList<QVariantMap> &added,
                                                   const QList<QVariantMap> &removed,
                                                   const QList<QVariantMap> &modified)
{
    Q_EMIT ThreadParticipantsChanged(thread, added, removed, modified);
}

QVariantMap HistoryServiceDBus::ThreadForProperties(const QString &accountId,
                                                    int type,
                                                    const QVariantMap &properties,
                                                    int matchFlags,
                                                    bool create)
{
    return HistoryDaemon::instance()->threadForProperties(accountId,
                                                            (History::EventType) type,
                                                            properties,
                                                            (History::MatchFlags) matchFlags,
                                                            create);
}

QVariantMap HistoryServiceDBus::ThreadForParticipants(const QString &accountId,
                                                      int type,
                                                      const QStringList &participants,
                                                      int matchFlags,
                                                      bool create)
{
    QVariantMap properties;
    properties[History::FieldParticipants] = participants;

    return HistoryDaemon::instance()->threadForProperties(accountId,
                                                            (History::EventType) type,
                                                            properties,
                                                            (History::MatchFlags) matchFlags,
                                                            create);
}

bool HistoryServiceDBus::WriteEvents(const QList<QVariantMap> &events)
{
    return HistoryDaemon::instance()->writeEvents(events, QVariantMap());
}

bool HistoryServiceDBus::RemoveThreads(const QList<QVariantMap> &threads)
{
    return HistoryDaemon::instance()->removeThreads(threads);
}

bool HistoryServiceDBus::RemoveEvents(const QList<QVariantMap> &events)
{
    return HistoryDaemon::instance()->removeEvents(events);
}

QString HistoryServiceDBus::QueryThreads(int type, const QVariantMap &sort, const QVariantMap &filter, const QVariantMap &properties)
{
    return HistoryDaemon::instance()->queryThreads(type, sort, filter, properties);
}

QString HistoryServiceDBus::QueryEvents(int type, const QVariantMap &sort, const QVariantMap &filter)
{
    return HistoryDaemon::instance()->queryEvents(type, sort, filter);
}

QVariantMap HistoryServiceDBus::GetSingleThread(int type, const QString &accountId, const QString &threadId, const QVariantMap &properties)
{
    return HistoryDaemon::instance()->getSingleThread(type, accountId, threadId, properties);
}

QVariantMap HistoryServiceDBus::GetSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    return HistoryDaemon::instance()->getSingleEvent(type, accountId, threadId, eventId);
}

void HistoryServiceDBus::timerEvent(QTimerEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__ << event->timerId();
    if (event->timerId() == mSignalsTimer) {
        killTimer(mSignalsTimer);
        mSignalsTimer = -1;
        processSignals();
    }
}

void HistoryServiceDBus::triggerSignals()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (mSignalsTimer >= 0) {
        killTimer(mSignalsTimer);
    }

    mSignalsTimer = startTimer(100);
}

void HistoryServiceDBus::processSignals()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mThreadsAdded.isEmpty()) {
        Q_EMIT ThreadsAdded(mThreadsAdded);
        mThreadsAdded.clear();
    }

    if (!mThreadsModified.isEmpty()) {
        Q_EMIT ThreadsModified(mThreadsModified);
        mThreadsModified.clear();
    }

    if (!mThreadsRemoved.isEmpty()) {
        Q_EMIT ThreadsRemoved(mThreadsRemoved);
        mThreadsRemoved.clear();
    }

    if (!mEventsAdded.isEmpty()) {
        Q_EMIT EventsAdded(mEventsAdded);
        mEventsAdded.clear();
    }

    if (!mEventsModified.isEmpty()) {
        Q_EMIT EventsModified(mEventsModified);
        mEventsModified.clear();
    }

    if (!mEventsRemoved.isEmpty()) {
        Q_EMIT EventsRemoved(mEventsRemoved);
        mEventsRemoved.clear();
    }
}


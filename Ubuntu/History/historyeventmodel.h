/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#ifndef HISTORYEVENTMODEL_H
#define HISTORYEVENTMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "historythreadmodel.h"

class HistoryEventModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(HistoryQmlFilter *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(HistoryQmlSort *sort READ sort WRITE setSort NOTIFY sortChanged)
    Q_PROPERTY(HistoryThreadModel::EventType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(bool matchContacts READ matchContacts WRITE setMatchContacts NOTIFY matchContactsChanged)
    Q_PROPERTY(bool canFetchMore READ canFetchMore NOTIFY canFetchMoreChanged)
    Q_ENUMS(Role)
public:
    enum Role {
        AccountIdRole = Qt::UserRole,
        ThreadIdRole,
        ParticipantsRole,
        TypeRole,
        EventIdRole,
        SenderIdRole,
        TimestampRole,
        DateRole,
        NewEventRole,
        PropertiesRole,
        TextMessageRole,
        TextMessageTypeRole,
        TextMessageStatusRole,
        TextReadTimestampRole,
        TextReadSubjectRole,
        TextMessageAttachmentsRole,
        CallMissedRole,
        CallDurationRole,
        LastRole
    };

    explicit HistoryEventModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    QVariant eventData(const History::Event &event, int role) const;

    Q_INVOKABLE virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE virtual void fetchMore(const QModelIndex &parent = QModelIndex());

    virtual QHash<int, QByteArray> roleNames() const;

    HistoryQmlFilter *filter() const;
    void setFilter(HistoryQmlFilter *value);

    HistoryQmlSort *sort() const;
    void setSort(HistoryQmlSort *value);

    HistoryThreadModel::EventType type() const;
    void setType(HistoryThreadModel::EventType value);

    bool matchContacts() const;
    void setMatchContacts(bool value);

    Q_INVOKABLE QString threadIdForParticipants(const QString &accountId,
                                                int eventType,
                                                const QStringList &participants,
                                                int matchFlags = (int)History::MatchCaseSensitive,
                                                bool create = false);

    Q_INVOKABLE bool removeEvent(const QString &accountId, const QString &threadId, const QString &eventId, int eventType);
    Q_INVOKABLE bool markEventAsRead(const QString &accountId, const QString &threadId, const QString &eventId, int eventType);

    Q_INVOKABLE bool removeEventAttachment(const QString &accountId, const QString &threadId, const QString &eventId, int eventType, const QString &attachmentId);
    Q_INVOKABLE virtual QVariant get(int row) const;

Q_SIGNALS:
    void countChanged();
    void filterChanged();
    void sortChanged();
    void typeChanged();
    void matchContactsChanged();
    void canFetchMoreChanged();

protected Q_SLOTS:
    void triggerQueryUpdate();
    virtual void updateQuery();
    virtual void onEventsAdded(const History::Events &events);
    virtual void onEventsModified(const History::Events &events);
    virtual void onEventsRemoved(const History::Events &events);
    void onContactInfoChanged(const QString &phoneNumber, const QVariantMap &contactInfo);

protected:
    void timerEvent(QTimerEvent *event);
    History::Events fetchNextPage();

private:
    History::EventViewPtr mView;
    History::Events mEvents;
    bool mCanFetchMore;
    HistoryQmlFilter *mFilter;
    HistoryQmlSort *mSort;
    HistoryThreadModel::EventType mType;
    bool mMatchContacts;
    QHash<int, QByteArray> mRoles;
    mutable QMap<History::TextEvent, QList<QVariant> > mAttachmentCache;
    History::Events mEventWritingQueue;
    int mEventWritingTimer;
    int mUpdateTimer;
};

#endif // HISTORYEVENTMODEL_H

/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "eventview.h"
#include "manager.h"
#include "thread.h"
#include "threadview.h"
#include "textevent.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)
Q_DECLARE_METATYPE(History::Threads)
Q_DECLARE_METATYPE(History::Events)

class ManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testThreadForParticipants_data();
    void testThreadForParticipants();
    void testQueryEvents();
    void testQueryThreads();
    void testGetSingleThread();
    void testWriteEvents();
    void cleanupTestCase();

private:
    History::Manager *mManager;
};

void ManagerTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();
    qRegisterMetaType<History::Threads>();
    qRegisterMetaType<History::Events>();
    mManager = History::Manager::instance();
}

void ManagerTest::testThreadForParticipants_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<History::EventType>("type");
    QTest::addColumn<QStringList>("participants");
    QTest::addColumn<History::MatchFlags>("matchFlags");
    QTest::addColumn<QStringList>("participantsToMatch");

    QTest::newRow("text thread with one participant") << "oneAccountId"
                                                      << History::EventTypeText
                                                      << (QStringList() << "oneParticipant")
                                                      << History::MatchFlags(History::MatchCaseSensitive)
                                                      << (QStringList() << "oneParticipant");
    QTest::newRow("voice thread using phone match") << "anotherAccountId"
                                                    << History::EventTypeVoice
                                                    << (QStringList() << "+1234567890")
                                                    << History::MatchFlags(History::MatchPhoneNumber)
                                                    << (QStringList() << "4567890");
}

void ManagerTest::testThreadForParticipants()
{
    QFETCH(QString, accountId);
    QFETCH(History::EventType, type);
    QFETCH(QStringList, participants);
    QFETCH(History::MatchFlags, matchFlags);
    QFETCH(QStringList, participantsToMatch);

    QSignalSpy spy(mManager, SIGNAL(threadsAdded(History::Threads)));
    History::Thread thread = mManager->threadForParticipants(accountId, type, participants, matchFlags, true);
    QVERIFY(!thread.isNull());
    QTRY_COMPARE(spy.count(), 1);

    QCOMPARE(thread.accountId(), accountId);
    QCOMPARE(thread.type(), type);
    QCOMPARE(thread.participants(), participants);

    // now try to get the thread again to see if it is returned correctly
    History::Thread sameThread = mManager->threadForParticipants(accountId, type, participantsToMatch, matchFlags, false);
    QVERIFY(sameThread == thread);
}

void ManagerTest::testQueryEvents()
{
    // just make sure the view returned is not null
    // the contents of the view will be tested in its own tests
    History::EventViewPtr eventView = mManager->queryEvents(History::EventTypeText);
    QVERIFY(!eventView.isNull());
    QVERIFY(eventView->isValid());
}

void ManagerTest::testQueryThreads()
{
    // just make sure the view returned is not null
    // the contents of the view will be tested in its own tests
    History::ThreadViewPtr threadView = mManager->queryThreads(History::EventTypeVoice);
    QVERIFY(!threadView.isNull());
    QVERIFY(threadView->isValid());
}

void ManagerTest::testGetSingleThread()
{
    History::Thread thread = mManager->threadForParticipants("theAccountId",
                                                             History::EventTypeText,
                                                             QStringList() << "theParticipant",
                                                             History::MatchCaseSensitive, true);
    QVERIFY(!thread.isNull());

    // try getting the same thread
    History::Thread sameThread = mManager->getSingleThread(thread.type(), thread.accountId(), thread.threadId());
    QVERIFY(sameThread == thread);
}

void ManagerTest::testWriteEvents()
{
    // create two threads, one for voice and one for text

    History::Thread textThread = mManager->threadForParticipants("textAccountId",
                                                                 History::EventTypeText,
                                                                 QStringList()<< "textParticipant",
                                                                 History::MatchCaseSensitive, true);
    History::Thread voiceThread = mManager->threadForParticipants("voiceAccountId",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< "voiceParticipant",
                                                                  History::MatchCaseSensitive, true);
    // insert some text and voice events
    History::Events events;
    for (int i = 0; i < 50; ++i) {
        History::TextEvent textEvent(textThread.accountId(),
                                     textThread.threadId(),
                                     QString("eventId%1").arg(i),
                                     textThread.participants().first(),
                                     QDateTime::currentDateTime(),
                                     true,
                                     QString("Hello world %1").arg(i),
                                     History::MessageTypeText,
                                     History::MessageFlags());
        events.append(textEvent);

        History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                       voiceThread.threadId(),
                                       QString("eventId%1").arg(i),
                                       voiceThread.participants().first(),
                                       QDateTime::currentDateTime(),
                                       true,
                                       true);
        events.append(voiceEvent);
    }

    QSignalSpy newEventsSpy(mManager, SIGNAL(eventsAdded(History::Events)));
    QSignalSpy threadsModifiedSpy(mManager, SIGNAL(threadsModified(History::Threads)));
    QVERIFY(mManager->writeEvents(events));
    QTRY_COMPARE(newEventsSpy.count(), 1);
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);

    // check that the signal was emitted with the correct number of events
    History::Events returnedEvents = newEventsSpy.first().first().value<History::Events>();

    // just in case, sort the lists before comparing
    qSort(events);
    qSort(returnedEvents);
    QCOMPARE(returnedEvents, events);

    History::Threads returnedThreads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(returnedThreads.count(), 2);

    // now modify two events and write them again to see if they are properly notified
    History::TextEvent modifiedTextEvent = events[0];
    History::VoiceEvent modifiedVoiceEvent = events[1];

    modifiedTextEvent.setNewEvent(false);
    modifiedTextEvent.setMessageFlags(History::MessageFlagDelivered);
    modifiedVoiceEvent.setNewEvent(false);

    QSignalSpy eventsModifiedSpy(mManager, SIGNAL(eventsModified(History::Events)));
    threadsModifiedSpy.clear();
    events.clear();
    events << modifiedTextEvent;
    events << modifiedVoiceEvent;
    QVERIFY(mManager->writeEvents(events));
    QTRY_COMPARE(eventsModifiedSpy.count(), 1);
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);

    returnedEvents = eventsModifiedSpy.first().first().value<History::Events>();
    qDebug() << returnedEvents.first().accountId();
    QCOMPARE(returnedEvents.count(), 2);

    returnedThreads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(returnedThreads.count(), 2);
}

void ManagerTest::cleanupTestCase()
{
    delete mManager;
}

QTEST_MAIN(ManagerTest)
#include "ManagerTest.moc"


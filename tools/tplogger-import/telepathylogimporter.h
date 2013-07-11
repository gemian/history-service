#ifndef TELEPATHYLOGIMPORTER_H
#define TELEPATHYLOGIMPORTER_H

#include <QObject>
#include <TelepathyLoggerQt/Types>
#include <TelepathyLoggerQt/CallEvent>
#include <TelepathyLoggerQt/TextEvent>
#include <Types>

class TelepathyLogImporter : public QObject
{
    Q_OBJECT
public:
    explicit TelepathyLogImporter(QObject *parent = 0);

public Q_SLOTS:
    void onCallEventLoaded(const Tpl::CallEventPtr &event);
    void onMessageEventLoaded(const Tpl::TextEventPtr &event);
    void onFinished();
private:
    History::WriterPtr mWriter;
    int mTextEvents;
    int mVoiceEvents;
};

#endif // TELEPATHYLOGIMPORTER_H
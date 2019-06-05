#ifndef AWB_FAKE_H
#define AWB_FAKE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QNetworkReply>
#include <QNetworkAccessManager>

namespace AceWebBrowser {

class FakeNetworkReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit FakeNetworkReply(QObject *parent = 0);

    void abort();
    qint64 readData(char* data, qint64 maxlen);
};

class FakeNetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit FakeNetworkManager(QObject *parent = 0);

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);
};

}

#endif // AWB_FAKE_H

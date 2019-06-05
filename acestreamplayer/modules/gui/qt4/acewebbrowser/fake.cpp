#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/fake.hpp"

using namespace AceWebBrowser;

FakeNetworkReply::FakeNetworkReply(QObject *parent) :
    QNetworkReply(parent)
{
}

void FakeNetworkReply::abort()
{
}

qint64 FakeNetworkReply::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data); Q_UNUSED(maxlen)
    return 0;
}


FakeNetworkManager::FakeNetworkManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

QNetworkReply *FakeNetworkManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    Q_UNUSED(op); Q_UNUSED(req); Q_UNUSED(outgoingData);
    return new FakeNetworkReply(this);
}

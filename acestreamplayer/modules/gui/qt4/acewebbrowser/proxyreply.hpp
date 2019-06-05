#ifndef AWB_PROXYREPLY_H
#define AWB_PROXYREPLY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QNetworkReply>

namespace AceWebBrowser {

class ProxyReply : public QNetworkReply
{
    Q_OBJECT
public:
    ProxyReply(QNetworkReply *reply, QObject *parent = 0);
    ~ProxyReply();

    void abort();
    void close();
    bool isSequential() const;

    void setReadBufferSize(qint64 size);

    virtual qint64 bytesAvailable() const;

    virtual qint64 bytesToWrite() const { return -1; }
    virtual bool canReadLine() const { return false; }

    virtual bool waitForReadyRead(int) { return false; }
    virtual bool waitForBytesWritten(int) { return false; }

    virtual qint64 readData(char* data, qint64 maxlen);

private:
    QNetworkReply *mRealReply;
    QByteArray mProxyBuffer;
    QByteArray mRealData;

public slots:
    void ignoreSslErrors();

    void metaDataChangedProxy();
    void readyReadProxy();
    void finishedProxy();
    void errorProxy(QNetworkReply::NetworkError _error);
};

}

#endif // AWB_PROXYREPLY_H

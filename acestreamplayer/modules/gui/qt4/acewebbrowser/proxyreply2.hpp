#ifndef AWB_PROXYREPLY2_H
#define AWB_PROXYREPLY2_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QNetworkReply>

namespace AceWebBrowser {

class Browser;
class ProxyReply2 : public QNetworkReply
{
    Q_OBJECT
public:
    ProxyReply2(QNetworkReply *reply, QNetworkAccessManager *filterNam, Browser *browser, QObject *parent = 0);
    ~ProxyReply2();

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
    bool mBlockRequest;
    bool mFinished;
    Browser *mBrowser;

public slots:
    void ignoreSslErrors();

    void metaDataChangedProxy();
    void readyReadProxy();
    void finishedProxy();
    void errorProxy(QNetworkReply::NetworkError _error);

private slots:
    void filterFinished();
};

}

#endif // AWB_PROXYREPLY2_H

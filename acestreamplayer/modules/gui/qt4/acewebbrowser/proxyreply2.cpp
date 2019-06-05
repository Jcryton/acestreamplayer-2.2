#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/proxyreply2.hpp"
#include "acewebbrowser/fake.hpp"
#include "acewebbrowser/browser.hpp"
#include "acewebbrowser/browsermanager.hpp"

#include <QNetworkCookie>
#include <QDebug>

using namespace AceWebBrowser;

ProxyReply2::ProxyReply2(QNetworkReply *reply, QNetworkAccessManager *filterNam, Browser *browser, QObject *parent) :
    QNetworkReply(parent)
  , mRealReply(reply)
  , mBlockRequest(false)
  , mFinished(false)
  , mBrowser(browser)
{
    qDebug() << "ProxyReply2 for" << mRealReply->url().toString();

    QString filterCacheKey = mRealReply->url().host() + mRealReply->url().path();
    if(!mBrowser->manager()->filterCacheHasValue(filterCacheKey)) {
        QString host = mBrowser->engineHost();
        int port = mBrowser->enginePort();
        if(!host.isEmpty() && port != 0) {
            QString filterUrl = QString("http://%1:%2/webui/checkurl?url=%3")
                    .arg(host)
                    .arg(QString::number(port))
                    .arg(QString(QUrl::toPercentEncoding(mRealReply->url().toString())));

            //QString filterUrl = "http://192.168.1.11/test/filter.php?url=" + QString(QUrl::toPercentEncoding(mRealReply->url().toString()));

            QNetworkReply *filterReply = filterNam->get(QNetworkRequest(QUrl(filterUrl)));
            QObject::connect(filterReply, SIGNAL(finished()), this, SLOT(filterFinished()));
        }
    }
    else {
        mBlockRequest = mBrowser->manager()->filterCacheGetValue(filterCacheKey);
    }

    setOperation(mRealReply->operation());
    setRequest(mRealReply->request());
    setUrl(mRealReply->url());

    connect(mRealReply, SIGNAL(metaDataChanged()), SLOT(metaDataChangedProxy()));
    connect(mRealReply, SIGNAL(readyRead()), SLOT(readyReadProxy()));
    connect(mRealReply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorProxy(QNetworkReply::NetworkError)));
    connect(mRealReply, SIGNAL(finished()), SLOT(finishedProxy()));

    connect(mRealReply, SIGNAL(uploadProgress(qint64,qint64)), SIGNAL(uploadProgress(qint64,qint64)));
    connect(mRealReply, SIGNAL(downloadProgress(qint64,qint64)), SIGNAL(downloadProgress(qint64,qint64)));

    setOpenMode(ReadOnly);
}

ProxyReply2::~ProxyReply2()
{
    delete mRealReply;
}

void ProxyReply2::filterFinished()
{
    if(mFinished) {
        qDebug() << "ProxyReply2::filterFinished: request is already finished: url =" << mRealReply->url().toString();
        return;
    }

    QNetworkReply *filterReply = qobject_cast<QNetworkReply*>(sender());
    if(filterReply) {
        int filterStatus = filterReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString readData = filterReply->readAll();

        qDebug() << "ProxyReply2::filterFinished: status" << filterStatus << "data" << readData << "url =" << mRealReply->url().toString();
        if(filterStatus == 200) {
            QString filterCacheKey = mRealReply->url().host() + mRealReply->url().path();
            mBrowser->manager()->filterCacheAddValue(filterCacheKey, !readData.compare("0"));
            if(!readData.compare("0")) {
                qDebug() << "ProxyReply2::filterFinished: block url" << mRealReply->url().toString();
                mBlockRequest = true;
            }
        }
        filterReply->deleteLater();
    }
}

void ProxyReply2::abort()
{
    mRealReply->abort();
}

void ProxyReply2::close()
{
    mRealReply->close();
}

bool ProxyReply2::isSequential() const
{
    return mRealReply->isSequential();
}

void ProxyReply2::setReadBufferSize(qint64 size)
{
    QNetworkReply::setReadBufferSize(size);
    mRealReply->setReadBufferSize(size);
}

qint64 ProxyReply2::bytesAvailable() const
{
    return mRealReply->bytesAvailable() + QIODevice::bytesAvailable();
}

qint64 ProxyReply2::readData(char *data, qint64 maxlen)
{
    return mRealReply->read(data, maxlen);
}

void ProxyReply2::ignoreSslErrors()
{
    mRealReply->ignoreSslErrors();
}

void ProxyReply2::metaDataChangedProxy()
{
    QList<QByteArray> headers = mRealReply->rawHeaderList();
    foreach(QByteArray header, headers)
        setRawHeader(header, mRealReply->rawHeader(header));

    setHeader(QNetworkRequest::ContentTypeHeader, mRealReply->header(QNetworkRequest::ContentTypeHeader));
    setHeader(QNetworkRequest::ContentLengthHeader, mRealReply->header(QNetworkRequest::ContentLengthHeader));
    setHeader(QNetworkRequest::LocationHeader, mRealReply->header(QNetworkRequest::LocationHeader));
    setHeader(QNetworkRequest::LastModifiedHeader, mRealReply->header(QNetworkRequest::LastModifiedHeader));
    setHeader(QNetworkRequest::SetCookieHeader, mRealReply->header(QNetworkRequest::SetCookieHeader));

    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, mRealReply->attribute(QNetworkRequest::HttpStatusCodeAttribute));
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, mRealReply->attribute(QNetworkRequest::HttpReasonPhraseAttribute));
    setAttribute(QNetworkRequest::RedirectionTargetAttribute, mRealReply->attribute(QNetworkRequest::RedirectionTargetAttribute));
    setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, mRealReply->attribute(QNetworkRequest::ConnectionEncryptedAttribute));
    setAttribute(QNetworkRequest::CacheLoadControlAttribute, mRealReply->attribute(QNetworkRequest::CacheLoadControlAttribute));
    setAttribute(QNetworkRequest::CacheSaveControlAttribute, mRealReply->attribute(QNetworkRequest::CacheSaveControlAttribute));
    setAttribute(QNetworkRequest::SourceIsFromCacheAttribute, mRealReply->attribute(QNetworkRequest::SourceIsFromCacheAttribute));
    setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, mRealReply->attribute(QNetworkRequest::DoNotBufferUploadDataAttribute));
    emit metaDataChanged();
}

void ProxyReply2::readyReadProxy()
{
    qDebug() << "ProxyReply2::readyReadProxy: url =" << mRealReply->url().toString() << "block =" << mBlockRequest << "finished =" << mFinished;

    if(mFinished) {
        return;
    }

    if(mBlockRequest) {
        mFinished = true;
        QNetworkReply::NetworkError _error = QNetworkReply::ContentNotFoundError;
        setError(_error, errorString());
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 404);
        setAttribute( QNetworkRequest::HttpReasonPhraseAttribute, "Request blocked");

        mRealReply->readAll();
        emit error(_error);
        emit finished();
    }
    else {
        emit readyRead();
    }
}

void ProxyReply2::finishedProxy()
{
    qDebug() << "ProxyReply2::finishedProxy: finished =" << mFinished << "url =" << mRealReply->url().toString();
    if(!mFinished) {
        mFinished = true;
        emit finished();
    }
}

void ProxyReply2::errorProxy(QNetworkReply::NetworkError _error)
{
    setError(_error, errorString());
    emit error(_error);
}


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/networkmanager.hpp"
#include "acewebbrowser/browser.hpp"
#include "acewebbrowser/cookiejar.hpp"
#include "acewebbrowser/cookiemanager.hpp"
#include "acewebbrowser/proxyreply.hpp"
#include "acewebbrowser/proxyreply2.hpp"

#include <QSslError>

using namespace AceWebBrowser;

NetworkManager::NetworkManager(Browser *browser, QObject *parent) :
    QNetworkAccessManager(parent)
  , mBrowser(browser)
  , mReferer("")
  , mFilterNAM(new QNetworkAccessManager(this))
{
    mTrustedSSLHosts.append("acestream.net");
    setCookieJar(CookieManager::getInstanse()->createCookieJar(mBrowser->cookiesType()));

#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));
#endif
}

void NetworkManager::setReferer(const QString &referer)
{
    mReferer = referer;
}

QNetworkReply *NetworkManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    QNetworkRequest request = req;
    if(!mReferer.isEmpty()) {
        qDebug() << "NetworkManager::createRequest referer:" << mReferer;
        request.setRawHeader("Referer", mReferer.toUtf8());
    }
    
    QString url = req.url().toString();

    QNetworkReply *reply = QNetworkAccessManager::createRequest(op, request, outgoingData);

    //TODO: replace with regex
    if(url.startsWith("http://www.youtube.com/watch?v=") || url.startsWith("https://www.youtube.com/watch?v=")) {
        return new ProxyReply(reply, this);
    }
    else if(mBrowser->urlFilter()) {
        return new ProxyReply2(reply, mFilterNAM, mBrowser, this);
    }
    else {
        return reply;
    }
}

#ifndef QT_NO_OPENSSL
void NetworkManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    QString replyHost = reply->url().host();
    QStringList errorStrings;
    for (int i = 0; i < error.count(); ++i)
        errorStrings += error.at(i).errorString();
    QString errors = errorStrings.join(QLatin1String("\n"));

    qWarning() << "NetworkManager::sslErrors:" << replyHost << reply->url().toString() << errors;

    foreach( QString s, mTrustedSSLHosts ) {
        if(replyHost.indexOf(s) != -1) {
            qWarning() << "NetworkManager::sslErrors: Ignore ssl errors for this domain";
            reply->ignoreSslErrors();
            break;
        }
    }
}
#endif

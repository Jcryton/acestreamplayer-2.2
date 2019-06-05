#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/proxyreply.hpp"
#include "acewebbrowser/fake.hpp"

#include <QNetworkCookie>
#include <QDebug>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>

using namespace AceWebBrowser;

ProxyReply::ProxyReply(QNetworkReply *reply, QObject *parent) :
    QNetworkReply(parent)
  , mRealReply(reply)
{
    qDebug("ProxyReply::ProxyReply: %p for %s", this, qPrintable(mRealReply->url().toString()) );

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

ProxyReply::~ProxyReply()
{
    delete mRealReply;
}

void ProxyReply::abort()
{
    mRealReply->abort();
}

void ProxyReply::close()
{
    mRealReply->close();
}

bool ProxyReply::isSequential() const
{
    return mRealReply->isSequential();
}

void ProxyReply::setReadBufferSize(qint64 size)
{
    QNetworkReply::setReadBufferSize(size);
    mRealReply->setReadBufferSize(size);
}

qint64 ProxyReply::bytesAvailable() const
{
    return mProxyBuffer.size() + QIODevice::bytesAvailable();
}

qint64 ProxyReply::readData(char *data, qint64 maxlen)
{
    qint64 size = qMin(maxlen, qint64(mProxyBuffer.size()));
    memcpy(data, mProxyBuffer.constData(), size);
    mProxyBuffer.remove(0, size);
    return size;
}

void ProxyReply::ignoreSslErrors()
{
    mRealReply->ignoreSslErrors();
}

void ProxyReply::metaDataChangedProxy()
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

void ProxyReply::readyReadProxy()
{
    QByteArray data = mRealReply->readAll();
    mRealData += data;
}
//#include <QTextCodec>
void ProxyReply::finishedProxy()
{
    QString contentType = mRealReply->header(QNetworkRequest::ContentTypeHeader).toString();
    QString url = mRealReply->url().toString();
    bool removeElements = false;
    
    // hack to switch between removing elements and hiding them
    if(url.contains("&r=0.78")) {
        removeElements = true;
    }

    qDebug() << "finishedProxy" << url << contentType << removeElements;

    if(contentType.indexOf("text/html") != -1) {
        //QTextCodec *codec = QTextCodec::codecForHtml(mRealData, QTextCodec::codecForName("UTF-8"));
        //QString html = codec->toUnicode(mRealData);
        //QString html = QString::fromAscii(mRealData);
        QWebPage fakePage;
        QWebElement div;
        QString html;
        bool html5 = false;

        html = QString::fromUtf8(mRealData);
        
        // detect html5 on youtube page
        if(html.indexOf("innerHTML = swf") == -1) {
            html5 = true;
        }
        qDebug() << "preprocessor: html5" << html5;
        
        if(!html5) {
            html = html.replace("var swf = ",
                                "ytplayer.config.args.html5=false;"
                                "encoded.push(\"controls=1\");"
                                "encoded.push(\"fs=0\");"
                                "encoded.push(\"rel=0\");"
                                "var swf = ");
            html = html.replace("allowfullscreen=\\\"true\\\"", "wmode=\\\"opaque\\\" allowfullscreen=\\\"false\\\"");
        }
        else {
            html = html.replace("\"html5\": true", "\"html5\": false");
        }
        html = html.replace("id=\"player-api\"", "id=\"yt-player-api\"");
        html = html.replace("document.getElementById(\"player-api\")", "document.getElementById(\"yt-player-api\")");
        fakePage.settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        fakePage.settings()->setAttribute(QWebSettings::JavaEnabled, false);
        fakePage.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
        fakePage.settings()->setAttribute(QWebSettings::PluginsEnabled, false);
        fakePage.settings()->setAttribute(QWebSettings::AutoLoadImages, false);
        fakePage.settings()->setAttribute(QWebSettings::LocalStorageEnabled, false);
        fakePage.setNetworkAccessManager(new FakeNetworkManager(this));
        fakePage.mainFrame()->setHtml(html);

        html = fakePage.mainFrame()->toHtml();
        
        if(html5) {
            // convert html5 page to flash version

            div = fakePage.mainFrame()->findFirstElement("script.html5player-ima");
            if(!div.isNull()) {
                div.removeFromDocument();
            }
            // remove html5 player
            div = fakePage.mainFrame()->findFirstElement("#movie_player");
            if(!div.isNull()) {
                div.removeFromDocument();
            }

            QWebElement body = fakePage.mainFrame()->findFirstElement("body");
            if(!body.isNull()) {
                body.appendInside("<script>\n"
                                  "(function() {"
                                  "if(window.ytplayer === undefined) {"
                                  "  return;"
                                  "}"
                                  "var encoded = [];"
                                  "for (var key in ytplayer.config.args) {"
                                    "encoded.push(encodeURIComponent(key) + '=' + encodeURIComponent(ytplayer.config.args[key]));"
                                  "}"
                                  "encoded.push(\"controls=1\");"
                                  "encoded.push(\"fs=0\");"
                                  "encoded.push(\"rel=0\");"
                                  "var swf = \"      \\u003cembed type=\\\"application\\/x-shockwave-flash\\\"     s\\u0072c=\\\"http:\\/\\/s.ytimg.com\\/yts\\/swfbin\\/player-vfl6hm3iS\\/watch_as3.swf\\\"     name=\\\"movie_player\\\"     id=\\\"movie_player\\\"    flashvars=\\\"__flashvars__\\\"  wmode=\\\"opaque\\\"   allowfullscreen=\\\"false\\\" allowscriptaccess=\\\"always\\\" bgcolor=\\\"#000000\\\"\\u003e\\n\";"
                                  "swf = swf.replace('__flashvars__', encoded.join('&'));"
                                  "document.getElementById('yt-player-api').innerHTML = swf;"
                                  "ytplayer.config.loaded = true"
                                  "})();"
                                  "\n</script>"
                            );
            }
        }

        // get like/dislike statuses before #content is removed
        // #watch-like-dislike-buttons should have 'liked' or 'disliked' class if user has rated video
        div = fakePage.mainFrame()->findFirstElement("#watch-like-dislike-buttons");
        if(!div.isNull()) {
            QString userRating = "";
            if(div.hasClass("liked")) {
                userRating = "liked";
            }
            if(div.hasClass("disliked")) {
                userRating = "disliked";
            }
            qDebug() << "finishedProxy: userRating" << userRating;

            if(userRating.length()) {
                QWebElement head = fakePage.mainFrame()->findFirstElement("head");
                if(!head.isNull()) {
                    head.appendInside("<script>"
                                      "var aceyt = {};"
                                      "aceyt.config = {\"user_rating\": \"" + userRating + "\"};"
                                      "</script>"
                                      );
                }
            }
        }
        
        div = fakePage.mainFrame()->findFirstElement("div#content");
        if(!div.isNull()) {
            qDebug() << "div found, remove from document";
            if(removeElements) {
                div.removeFromDocument();
            }
            else {
                div.setStyleProperty("display", "none");
            }
        }
        else {
            qDebug() << "div not found";
        }

        div = fakePage.mainFrame()->findFirstElement("#footer-container");
        if(!div.isNull()) {
            qDebug() << "div found, remove from document";
            if(removeElements) {
                div.removeFromDocument();
            }
            else {
                div.setStyleProperty("display", "none");
            }
        }
        else {
            qDebug() << "div not found";
        }

        div = fakePage.mainFrame()->findFirstElement("#guide");
        if(!div.isNull()) {
            qDebug() << "div found, remove from document";
            if(removeElements) {
                div.removeFromDocument();
            }
            else {
                div.setStyleProperty("display", "none");
            }
        }
        else {
            qDebug() << "div not found";
        }

        div = fakePage.mainFrame()->findFirstElement("#yt-masthead-container");
        if(!div.isNull()) {
            qDebug() << "div found, remove from document";
            if(removeElements) {
                div.removeFromDocument();
            }
            else {
                div.setStyleProperty("display", "none");
            }
        }
        else {
            qDebug() << "div not found";
        }

        div = fakePage.mainFrame()->findFirstElement("#sb-wrapper");
        if(!div.isNull()) {
            qDebug() << "div found, remove from document";
            if(removeElements) {
                div.removeFromDocument();
            }
            else {
                div.setStyleProperty("display", "none");
            }
        }

        div = fakePage.mainFrame()->findFirstElement("#alerts");
        if(!div.isNull()) {
            qDebug() << "div found, remove from document";
            if(removeElements) {
                div.removeFromDocument();
            }
            else {
                div.setStyleProperty("display", "none");
            }
        }

        div = fakePage.mainFrame()->findFirstElement("#yt-player-api");
        if(!div.isNull()) {
            qDebug() << "player found";
            //div.setAttribute("id", "yt-player-api");
            div.setStyleProperty("position", "fixed");
            div.setStyleProperty("left", "0px");
            div.setStyleProperty("top", "0px");
            div.setStyleProperty("width", "100%");
            div.setStyleProperty("height", "100%");
        }
        else {
            qDebug() << "player not found";
        }
        html = fakePage.mainFrame()->toHtml();
        mProxyBuffer = html.toUtf8();
    }
    else {
        mProxyBuffer = mRealData;
    }

    emit readyRead();
    emit finished();
}

void ProxyReply::errorProxy(QNetworkReply::NetworkError _error)
{
    setError(_error, errorString());
    emit error(_error);
}

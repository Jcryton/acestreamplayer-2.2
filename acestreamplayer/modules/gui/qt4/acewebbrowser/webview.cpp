#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/webview.hpp"
#include "acewebbrowser/webpage.hpp"
#include "acewebbrowser/browser.hpp"
#include "acewebbrowser/javascriptobject.hpp"
#include "acewebbrowser/networkmanager.hpp"
#include "acewebbrowser/dummy.hpp"

#include <QNetworkReply>
#include <QWebFrame>
#include <QWebElement>

using namespace AceWebBrowser;

WebView::WebView(QWidget *parent) :
    QWebView(parent)
  , mWebPage(new WebPage(this))
  , mJSWindowObject(0)
  , mDelegationPolicy(QWebPage::DelegateAllLinks)
  , mLastChangedUrl("")
{
    setPage(mWebPage);

    QPalette pal = palette();
    mDefaultBgColor = pal.color(QPalette::Base);
    pal.setColor(QPalette::Base, Qt::black);
    page()->setPalette(pal);

    settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);

    //settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, SIGNAL(urlChanged(const QUrl&)), SLOT(pageUrlChanged(const QUrl&)));
    connect(page()->mainFrame(), SIGNAL(loadStarted()), SLOT(pageLoadStarted()));
    connect(page()->mainFrame(), SIGNAL(loadFinished(bool)), SLOT(pageLoadFinished(bool)));
    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(slotJavaScriptWindowObjectCleared()));
    connect(page(), SIGNAL(frameCreated(QWebFrame*)), SLOT(pageFrameCreated(QWebFrame*)));
}

void WebView::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "WebView::focusInEvent";
    emit gotFocus();
}

void WebView::load(const QUrl& url)
{
    mLastChangedUrl = url;
    QWebView::load(url);
}

void WebView::changeReferer(WebView::RefererType refType)
{
    if(page()->networkAccessManager()) {
        NetworkManager *nm = qobject_cast<NetworkManager*>(page()->networkAccessManager());
        nm->setReferer(refType == RT_CURRENT ? url().toString() : "");
    }
}

void WebView::setupNetworkAccessManager()
{
    connect(page()->networkAccessManager(), SIGNAL(finished(QNetworkReply*)), SLOT(networkAccessManagerFinished(QNetworkReply*)));
}

void WebView::setJavaScriptWindowObject(QObject *object)
{
    mJSWindowObject = object;
}

void WebView::setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy)
{
    mDelegationPolicy = policy;
}

QWebPage::LinkDelegationPolicy WebView::linkDelegationPolicy()
{
    return mDelegationPolicy;
}

void WebView::setScrollbarsEnable(bool enable)
{
    if(enable) {
        page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
        page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
    }
    else {
        page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    }
}

QWebView *WebView::createWindow(QWebPage::WebWindowType type)
{
    Q_UNUSED(type)
    Browser *parentBrowser = qobject_cast<Browser *>(parentWidget());
    if(parentBrowser && !parentBrowser->allowWindowOpen()) {
        qDebug() << "WebView::createWindow: creating new windows disabled";
        return NULL;
    }
    if(mDelegationPolicy == QWebPage::DelegateAllLinks) {
        // hack for window.open(...);
        DummyBrowser *dummy = new DummyBrowser(this);
        return dummy;
    }
    else {
        // open in current window
        setScrollbarsEnable(true);
        emit windowOpenedInSameBrowser();
        return this;
    }
}

void WebView::connectEmbedElements(QWebFrame *frame)
{
    qDebug() << "WebView::connectEmbedElements";
    if(mJSWindowObject) {
       frame->addToJavaScriptWindowObject("AceStreamBrowser", mJSWindowObject);
    }
    Browser *parentBrowser = qobject_cast<Browser *>(parentWidget());
    if(parentBrowser) {
        QStringList embedScripts = parentBrowser->embedScripts();
        foreach(QString script, embedScripts) {
            if(script.compare("")) {
                QString js = QString(ADD_JS_SCRIPT_JS).replace("%SCRIPT_URL%", script);
                frame->evaluateJavaScript(js);
            }
        }

        QString embedCode = parentBrowser->embedCode();
        if(embedCode.compare("")) {
            embedCode = embedCode.replace("'", "\\'");
            QString js = QString(ADD_JS_CODE_JS).replace("%SCRIPT_CODE%", embedCode);
            frame->evaluateJavaScript(js);
        }
    }
}

void WebView::pageUrlChanged(const QUrl &url)
{
    mLastChangedUrl = url;
    qDebug() << "WebView::pageUrlChanged: Url changed" << url.toString();
}

void WebView::pageLoadStarted()
{
    qDebug() << "WebView::pageLoadStarted: url" << page()->currentFrame()->url() << "requested" << page()->currentFrame()->requestedUrl();
}

void WebView::slotJavaScriptWindowObjectCleared()
{
    qDebug() << "WebView::slotJavaScriptWindowObjectCleared: url" << page()->currentFrame()->url() << "requested" << page()->currentFrame()->requestedUrl();
    connectEmbedElements(page()->currentFrame());
}

void WebView::pageLoadFinished(bool status)
{
    qDebug() << "WebView::pageLoadFinished: status" << status << "url" << page()->currentFrame()->url();
    if(status) {
        QPalette pal = palette();
        pal.setColor(QPalette::Base, mDefaultBgColor);
        page()->setPalette(pal);
        repaint();
        //connectEmbedElements(page()->currentFrame());
    }
    /*else {
        emit pageNetworkFinished(false);
    }*/
}

void WebView::pageFrameCreated(QWebFrame *frame)
{
    connect(frame, SIGNAL(loadFinished(bool)), SLOT(pageFrameLoadFinished(bool)));
}

void WebView::pageFrameLoadFinished(bool status)
{
    qDebug() << "WebView::pageFrameLoadFinished: status" << status;
    if(status) {
        QWebFrame *senderFrame = qobject_cast<QWebFrame*>(QObject::sender());
        if(senderFrame) {
            connectEmbedElements(senderFrame);
        }
    }
}

void WebView::networkAccessManagerFinished(QNetworkReply *reply)
{
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "WebView::networkAccessManagerFinished: url" << reply->url().toString() << "status" << httpStatus << "lastUrl" << mLastChangedUrl.toString();
    if(reply->error() != QNetworkReply::NoError)
    {
        QString httpStatusMessage = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        qWarning() << "Failed to load:" << reply->url() << ":" << QString::number(httpStatus) << ":" << httpStatusMessage;
    }
    
    if(reply->url() == mLastChangedUrl) {
        if(200 == httpStatus) {
            emit pageNetworkFinished(true);
        }
        else if(httpStatus == 0 || httpStatus >= 400) {
            emit pageNetworkFinished(false);
        }
    }
}

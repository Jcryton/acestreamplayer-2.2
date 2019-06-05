#ifndef AWB_WEBVIEW_H
#define AWB_WEBVIEW_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QWebView>
#include <QSslError>

namespace AceWebBrowser {

class WebPage;
class WebView : public QWebView
{
    Q_OBJECT
public:
    enum RefererType {
        RT_CURRENT,
        RT_BLANK
    };

public:
    explicit WebView(QWidget *parent = 0);

    void setJavaScriptWindowObject(QObject *object);
    void setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy);
    QWebPage::LinkDelegationPolicy linkDelegationPolicy();
    void setScrollbarsEnable(bool enable);
    void setupNetworkAccessManager();
    void load(const QUrl& url);
    void changeReferer(RefererType refType);

protected:
    QWebView *createWindow(QWebPage::WebWindowType type);
    void focusInEvent(QFocusEvent *event);

private:
    void connectEmbedElements(QWebFrame *frame);

private:
    WebPage *mWebPage;
    QObject *mJSWindowObject;
    QWebPage::LinkDelegationPolicy mDelegationPolicy;
    QUrl mLastChangedUrl;
    QColor mDefaultBgColor;

signals:
    void windowOpenedInSameBrowser();
    void pageNetworkFinished(bool);
    void gotFocus();

private slots:
    void pageUrlChanged(const QUrl &url);
    void pageLoadStarted();
    void pageLoadFinished(bool status);
    void pageFrameCreated(QWebFrame *frame);
    void pageFrameLoadFinished(bool status);
    void networkAccessManagerFinished(QNetworkReply *reply);
    void slotJavaScriptWindowObjectCleared();
};

}

#endif // AWB_WEBVIEW_H

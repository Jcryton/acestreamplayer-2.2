#ifndef AWB_DUMMY_H
#define AWB_DUMMY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QWebPage>
#include <QWebView>

namespace AceWebBrowser {

class DummyWebPage : public QWebPage
{
    Q_OBJECT
public:
    DummyWebPage(QObject *parent = 0) : QWebPage(parent) {}
    QString userAgentForUrl( const QUrl &url) const;

protected:
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);

signals:
    void openExternal();
};

class DummyBrowser : public QWebView
{
    Q_OBJECT
public:
    DummyBrowser(QWidget *parent = 0);
};

}

#endif // AWB_DUMMY_H

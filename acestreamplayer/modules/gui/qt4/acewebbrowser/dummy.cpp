#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/dummy.hpp"
#include "acewebbrowser/defines.hpp"
#include <QDesktopServices>
#include <QNetworkRequest>

using namespace AceWebBrowser;

QString DummyWebPage::userAgentForUrl(const QUrl &url) const
{
    Q_UNUSED(url)
    return USER_AGENT;
}

bool DummyWebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, QWebPage::NavigationType type)
{
    if(!request.url().isEmpty()) {
        QDesktopServices::openUrl(request.url());
        emit openExternal();
        return false;
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);
}

DummyBrowser::DummyBrowser(QWidget *parent) :
    QWebView(parent)
{
    setPage(new DummyWebPage(this));
    connect(page(), SIGNAL(openExternal()), SLOT(deleteLater()));
}


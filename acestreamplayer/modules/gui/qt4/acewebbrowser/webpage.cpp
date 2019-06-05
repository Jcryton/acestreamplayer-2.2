#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/webpage.hpp"

#include <QMessageBox>

using namespace AceWebBrowser;

WebPage::WebPage(QObject *parent) :
    QWebPage(parent)
  , mAllowDialogs(false)
  , mDialogsCanBeShown(false)
  , mHostUserAgent("")
  , mUserAgentType(BUA_OUR)
{
    setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
}

QString WebPage::userAgentForUrl(const QUrl &url) const
{
    Q_UNUSED(url)
    return mUserAgentType == BUA_HOST ? mHostUserAgent : USER_AGENT;
}

void WebPage::setAllowDialogs(bool allow)
{
    mAllowDialogs = allow;
}

void WebPage::setDialogsCanBeShown(bool can)
{
    mDialogsCanBeShown = mAllowDialogs && can;
}

void WebPage::setUserAgentType(BrowserUserAgent type)
{
    mUserAgentType = type;
}

void WebPage::setHostUserAgent(const QString &ua)
{
    mHostUserAgent = ua;
}

void WebPage::javaScriptAlert(QWebFrame *originatingFrame, const QString &msg)
{
    Q_UNUSED(originatingFrame);
    if(mDialogsCanBeShown) {
        qDebug() << "WebPage::javaScriptAlert: Showing alert from js";
        QMessageBox mb;
        mb.setText(msg);
        mb.exec();
    }
    else
        qDebug() << "WebPage::javaScriptAlert: Blocking alert from js";
}

bool WebPage::javaScriptConfirm(QWebFrame *originatingFrame, const QString &msg)
{
    Q_UNUSED(originatingFrame);
    bool result = true;
    if(mDialogsCanBeShown) {
        qDebug() << "WebPage::javaScriptConfirm: Showing confirm from js";
        QMessageBox mb;
        mb.addButton(QMessageBox::Yes);
        mb.addButton(QMessageBox::No);
        mb.setText(msg);
        result = (mb.exec() == QMessageBox::Yes);
    }
    else
        qDebug() << "WebPage::javaScriptConfirm: Blocking confirm from js";
    return result;
}

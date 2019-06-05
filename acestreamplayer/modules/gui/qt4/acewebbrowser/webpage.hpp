#ifndef AWB_WEBPAGE_H
#define AWB_WEBPAGE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QWebPage>
#include "acewebbrowser/defines.hpp"

namespace AceWebBrowser {

class WebPage : public QWebPage
{
    Q_OBJECT
public:
    explicit WebPage(QObject *parent = 0);

    QString userAgentForUrl (const QUrl & url) const;

    void setAllowDialogs(bool allow);
    void setDialogsCanBeShown(bool can);
    void setUserAgentType(BrowserUserAgent type);
    void setHostUserAgent(const QString& ua);

protected:
    void javaScriptAlert(QWebFrame *originatingFrame, const QString &msg);
    bool javaScriptConfirm(QWebFrame *originatingFrame, const QString &msg);

private:
    bool mAllowDialogs;
    bool mDialogsCanBeShown;
    BrowserUserAgent mUserAgentType;
    QString mHostUserAgent;
};

}

#endif // AWB_WEBPAGE_H

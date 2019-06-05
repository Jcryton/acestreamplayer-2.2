#ifndef AWB_COOKIEJAR_H
#define AWB_COOKIEJAR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QNetworkCookieJar>

#include "acewebbrowser/defines.hpp"

namespace AceWebBrowser {

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieJar(QObject *parent = 0);

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const;
    bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url);

    BrowserCookies browserCookiesType() const;
    void setBrowserCookiesType(BrowserCookies type);

private:
    bool mIsLoaded;
    BrowserCookies mCookiesType;

private slots:
    void updateCookies();
};

}

#endif // AWB_COOKIEJAR_H

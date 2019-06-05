#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/cookiejar.hpp"
#include "acewebbrowser/cookiemanager.hpp"

using namespace AceWebBrowser;

CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
  , mIsLoaded(false)
  , mCookiesType(BCOOK_OUR)
{
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    CookieJar *cj = const_cast<CookieJar*>(this);
    if (!mIsLoaded)
        cj->updateCookies();
    return QNetworkCookieJar::cookiesForUrl(url);
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!mIsLoaded) {
        updateCookies();
    }

    if(mCookiesType != BCOOK_NONE) {
        QString defaultDomain = QLatin1Char('.') + url.host();
        QString pathAndFileName = url.path();
        QString defaultPath = pathAndFileName.left(pathAndFileName.lastIndexOf(QLatin1Char('/'))+1);
        if (defaultPath.isEmpty())
            defaultPath = QLatin1Char('/');

        foreach(QNetworkCookie cookie, cookieList)
        {
            QList<QNetworkCookie> lst;

            if(cookie.path().isEmpty())
                cookie.setPath(defaultPath);

            if(cookie.domain().isEmpty()) {
                cookie.setDomain(defaultDomain);
            }
            else if(!cookie.domain().startsWith(QLatin1Char('.'))) {
                cookie.setDomain(QLatin1Char('.') + cookie.domain());
            }

            lst += cookie;
            QNetworkCookieJar::setCookiesFromUrl(lst, url);
            CookieManager::getInstanse()->insertCookie(cookie);
        }
        return true;
    }
    else {
        return false;
    }
}

BrowserCookies CookieJar::browserCookiesType() const
{
    return mCookiesType;
}

void CookieJar::setBrowserCookiesType(BrowserCookies type)
{
    mCookiesType = type;
}

void CookieJar::updateCookies()
{
    QList<QNetworkCookie> cookies;
    switch(mCookiesType) {
    case BCOOK_OUR:
        cookies = CookieManager::getInstanse()->getCookies("def");
        break;
    case BCOOK_ALL:
        cookies = CookieManager::getInstanse()->getCookies();
        break;
    case BCOOK_NONE:
    default:
        break;
    }
    setAllCookies(cookies);
    mIsLoaded = true;
}



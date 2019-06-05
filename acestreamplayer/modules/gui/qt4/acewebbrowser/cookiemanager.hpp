#ifndef AWB_COOKIEMANAGER_H
#define AWB_COOKIEMANAGER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "acewebbrowser/defines.hpp"

#include <QMap>
#include <QMultiHash>
#include <QPair>
#include <QMutex>
#include <QDateTime>
#include <QNetworkCookie>

namespace AceWebBrowser {

class CookieJar;
class CookieManager : public QObject
{
    Q_OBJECT

public:
    static CookieManager *getInstanse(QObject *parent=0);
    static void holdInstanse(const CookieManager *inst);
    static void releaseInstance(const CookieManager *inst);

    CookieJar *createCookieJar(BrowserCookies = BCOOK_OUR);
    QList<QNetworkCookie> getCookies(const QString& key = "");
    void insertCookie(QNetworkCookie cookie);

private:
    explicit CookieManager(QObject *parent=0);
    ~CookieManager();

    void load();
    inline void loadList(QDataStream &stream);
    void save(bool clearSessionCookies = false);
    inline void saveList(QDataStream &stream, const QString &key, const QList<QNetworkCookie> &list);

    void insertCookies(const QString &key, const QList<QNetworkCookie> &list);
    void insertCookie(const QString &key, const QNetworkCookie &cookie);
    QMultiHash<QString, QNetworkCookie>::iterator findCookie(const QNetworkCookie &value);
    void deleteKey(const QString &key);
    void clearCookies(bool clearSessionCookies);

private:
    static CookieManager *instance;
    static QMutex objectLock;
    static int holdersSize;

    QMutex mWriteLock;
    QMultiHash<QString, QNetworkCookie> mCookies;
    QDateTime mLastImport;

signals:
    void notifyChanged();
    void updateBrowserCookies(QList<QNetworkCookie> list, const QString &key, bool thatisall);

private slots:
    void updateExternalCookies(QList<QNetworkCookie> list, const QString &key, bool commit);
};

}

#endif // AWB_COOKIEMANAGER_H

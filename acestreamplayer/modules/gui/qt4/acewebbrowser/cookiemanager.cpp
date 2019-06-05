#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/cookiemanager.hpp"
#include "acewebbrowser/cookiejar.hpp"

#include "qt4.hpp"

#include <QDir>
#include <QFile>

using namespace AceWebBrowser;

CookieManager *CookieManager::instance = 0;
QMutex CookieManager::objectLock;
int CookieManager::holdersSize = 0;

CookieManager *CookieManager::getInstanse(QObject *obj)
{
    objectLock.lock();
    if(!instance) {
        instance = new CookieManager(obj);
    }
    objectLock.unlock();
    return instance;
}

void CookieManager::holdInstanse(const CookieManager *inst)
{
    if(instance && instance == inst) {
        objectLock.lock();
        holdersSize++;
        qDebug() << "CookieManager::holdInstanse: holders" << holdersSize;
        objectLock.unlock();
    }
}

void CookieManager::releaseInstance(const CookieManager *inst)
{
    if(instance && instance == inst) {
        objectLock.lock();
        holdersSize--;
        qDebug() << "CookieManager::releaseInstance: holders" << holdersSize;
        if(holdersSize <= 0 && instance) {
            delete instance;
            instance = 0;
            holdersSize = 0;
        }
        objectLock.unlock();
    }
}

CookieJar *CookieManager::createCookieJar(BrowserCookies cookieType)
{
    CookieJar *cj = new CookieJar();
    cj->setBrowserCookiesType(cookieType);
    connect(this, SIGNAL(notifyChanged()), cj, SLOT(updateCookies()));
    return cj;
}

QList<QNetworkCookie> CookieManager::getCookies(const QString& key)
{
    return (key == "") ? (QList<QNetworkCookie>)(mCookies.values()) : (QList<QNetworkCookie>)(mCookies.values(key));
}

void CookieManager::insertCookie(QNetworkCookie cookie)
{
    mWriteLock.lock();
    insertCookie("def", cookie);
    mWriteLock.unlock();
    save();
}

CookieManager::CookieManager(QObject *parent) :
    QObject(parent)
{
    load();
}

CookieManager::~CookieManager()
{
    save(true);
    mCookies.clear();
    qDebug() << "CookieManager::~CookieManager";
}

void CookieManager::load()
{
    QMutexLocker ml(&mWriteLock);

    char *psz_datadir = config_GetUserDir( VLC_DATA_DIR );
    char *psz_file;
    if(asprintf(&psz_file, "%s" DIR_SEP "cookies", psz_datadir) == -1)
        psz_file = NULL;
    free(psz_datadir);
    if(psz_file == NULL) return;
    QString filename(psz_file);
    free(psz_file);
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_7);

    loadList(in); // def
    loadList(in); // ff
    loadList(in); // ch
    loadList(in); // opwk

    in >> mLastImport;
}

void CookieManager::loadList(QDataStream &stream)
{
    QString key;
    stream >> key;
    quint32 cooksize;
    stream >> cooksize;
    QList<QNetworkCookie> list;
    for(quint32 i = 0; i < cooksize; ++i)
    {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> ck = QNetworkCookie::parseCookies(value);
        list.append(ck);
        if(stream.atEnd()) break;
    }

    insertCookies(key, list);
}

void CookieManager::save(bool clearSessionCookies)
{
    QMutexLocker ml(&mWriteLock);
    clearCookies(clearSessionCookies);

    char *psz_datadir = config_GetUserDir( VLC_DATA_DIR );
    char *psz_file;
    if(asprintf(&psz_file, "%s" DIR_SEP "cookies", psz_datadir) == -1)
        psz_file = NULL;
    free(psz_datadir);
    
    if(!psz_file) return;
    QString filename(psz_file);
    free(psz_file);
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly))
        return;

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_7);


    saveList(out, "def", mCookies.values("def"));
    saveList(out, "ff", mCookies.values("ff"));
    saveList(out, "ch", mCookies.values("ch"));
    saveList(out, "opwk", mCookies.values("opwk"));
    out << mLastImport;
    file.close();
}

void CookieManager::saveList(QDataStream &stream, const QString &key, const QList<QNetworkCookie> &list)
{
    stream << key;
    stream << (quint32)list.size();
    foreach (QNetworkCookie ck, list) {
        stream << ck.toRawForm();
    }
}

void CookieManager::insertCookies(const QString &key, const QList<QNetworkCookie> &list)
{
    foreach (QNetworkCookie value, list)
    {
        QMultiHash<QString, QNetworkCookie>::iterator it = findCookie(value);
        if(it != mCookies.end())
        {
            const QNetworkCookie &dup = (const QNetworkCookie)(it.value());
            if( dup.expirationDate().isValid()
                    && dup.expirationDate() > value.expirationDate() )
            {
                continue;
            }
            mCookies.erase(it);
        }
        mCookies.insert(key, value);
    }
}

void CookieManager::insertCookie(const QString &key, const QNetworkCookie &cookie)
{
    QMultiHash<QString, QNetworkCookie>::iterator it = findCookie(cookie);
    if(it != mCookies.end())
    {
        mCookies.erase(it);
    }
    mCookies.insert(key, cookie);
    emit notifyChanged();
}

QMultiHash<QString, QNetworkCookie>::iterator CookieManager::findCookie(const QNetworkCookie &value)
{
    QMultiHash<QString, QNetworkCookie>::iterator it;
    for(it = mCookies.begin(); it != mCookies.end(); ++it)
    {
        const QNetworkCookie &cur = (const QNetworkCookie)(it.value());
        if (value.name() == cur.name() &&
            value.domain() == cur.domain() &&
            value.path() == cur.path() &&
            value.isSecure() == cur.isSecure() &&
            value.isHttpOnly() == cur.isHttpOnly())
        {
            break;
        }
    }
    return it;
}

void CookieManager::deleteKey(const QString &key)
{
    mCookies.remove(key);
}

void CookieManager::clearCookies(bool clearSessionCookies)
{
    QDateTime now = QDateTime::currentDateTime();
    QMultiHash<QString, QNetworkCookie>::iterator it;
    for(it = mCookies.begin(); it != mCookies.end();)
    {
        const QNetworkCookie &cur = (const QNetworkCookie)(it.value());
        if( cur.isSessionCookie() && clearSessionCookies )
            it = mCookies.erase(it);
        else
        {
            if( !cur.isSessionCookie() && cur.expirationDate() < now )
                it = mCookies.erase(it);
            else
                ++it;
        }
    }
}

void CookieManager::updateExternalCookies(QList<QNetworkCookie> list, const QString &key, bool commit)
{
    if(list.size() > 0)
    {
        mWriteLock.lock();
        deleteKey(key);
        insertCookies(key, list);
        mWriteLock.unlock();

        if(commit)
        {
            mLastImport = QDateTime::currentDateTime();
            save();
            emit notifyChanged();
        }
    }
}

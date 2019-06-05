#ifndef AWB_LOADITEM_H
#define AWB_LOADITEM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "acewebbrowser/defines.hpp"

#include <QStringList>

namespace AceWebBrowser {

class UrlWithId {
public:
    UrlWithId() {}
    UrlWithId(QStringList _url, QStringList _id)
        : urls(_url), ids(_id), index(0) {}

    QString url() const { return urls.at(index); }
    QString urlsString() const { return urls.join(";"); }
    QString id() const { return ids.at(index); }
    QString idsString() const { return ids.join(";"); }
    bool next() const {
        if(index < urls.size() - 1) { ++index; return true; }
        else { return false; }
    }

    bool operator==(const UrlWithId &other) const {
        return !urls.join(".").compare(other.urls.join(".")) && !ids.join(".").compare(other.ids.join("."));
    }
    bool operator!=(const UrlWithId &other) const {
        return urls.join(".").compare(other.urls.join(".")) || ids.join(".").compare(other.ids.join("."));
    }

private:
    QStringList urls;
    QStringList ids;
    mutable int index;
};

class LoadItem
{
public:
    LoadItem();
    LoadItem(BrowserType _type, UrlWithId _uwid,
            int _w, int _h, int _l, int _t, int _r, int _b,
            bool _allowD, bool _enableF, BrowserCookies _cook, QStringList _embedS, QString _embedC,
            bool _preload,
            QString _contentT, QString _creativeT, QString _clickU,
            BrowserUserAgent _uA, int _cA, int _sT, bool _sH, bool _allowWO, int _group, bool _uF, bool _uIE);
    LoadItem(const LoadItem &other);
    ~LoadItem();

    bool operator==(const LoadItem &other) const;
    bool operator!=(const LoadItem &other) const;

    BrowserType type() const;
    const UrlWithId *urlWithId() const;
    int width() const;
    int height() const;
    int left() const;
    int top() const;
    int right() const;
    int bottom() const;
    bool allowDialogs() const;
    bool allowWindowOpen() const;
    bool enableFlash() const;
    BrowserCookies cookies() const;
    QStringList embedScripts() const;
    QString embedCode() const;
    bool preload() const;
    QString contentType() const;
    QString creativeType() const;
    QString clickUrl() const;
    BrowserUserAgent userAgent() const;
    int closeAfterSeconds() const;
    int showTime() const;
    bool startHidden() const;
    bool urlFilter() const;

    bool useIE() const;
    
    QString hostUserAgent() const;
    unsigned int fixedBottomSpace() const;
    unsigned int fixedFullscreenBottomSpace() const;
    bool shownRegistered() const;
    bool hideRegistered() const;
    bool completeRegistered() const { return mEventCompleteRegistered; }

    void setHostUserAgent(const QString& ua);
    void setFixedBottomSpace(unsigned int space);
    void setFixedFullscreenBottomSpace(unsigned int space);
    void setShownRegistered(bool val);
    void setHideRegistered(bool val);
    void setCompleteRegistered(bool val) {
        mEventCompleteRegistered = val;
    }
    void clearEventFlags();

    void setSize(int w, int h);

    void clear();
    bool isCleared() const;

    void setEngineHttpHost(const QString&);
    void setEngineHttpPort(int);
    QString engineHttpHost() const;
    int engineHttpPort() const;

    QString toString() const;
    int groupId() const;

private:
    BrowserType mType;
    UrlWithId mUrlWithId;
    int mWidth;
    int mHeight;
    int mLeft;
    int mTop;
    int mRight;
    int mBottom;
    bool mAllowDialogs;
    bool mAllowWindowOpen;
    bool mEnableFlash;
    BrowserCookies mCookies;
    QStringList mEmbedScripts;
    QString mEmbedCode;
    bool mPreload;
    QString mContentType;
    QString mCreativeType;
    QString mClickUrl;
    BrowserUserAgent mUserAgent;
    int mCloseAfterSeconds;
    int mShowTime;
    bool mStartHidden;
    bool mUrlFilter;

    //additional
    QString mHostUserAgent;
    unsigned int mFixedBottomSpace;
    unsigned int mFixedFullscreenBottomSpace;
    bool mEventShownRegistered;
    bool mEventHideRegistered;
    bool mEventCompleteRegistered;
    
    QString mEngineHttpHost;
    int mEngineHttpPort;

    int mGroupId;
    
    bool mUseIE;
};

}

Q_DECLARE_METATYPE(AceWebBrowser::LoadItem)

QDebug operator<<(QDebug debug, const AceWebBrowser::LoadItem &item);

#endif // AWB_LOADITEM_H

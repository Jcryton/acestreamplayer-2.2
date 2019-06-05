#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/loaditem.hpp"

using namespace AceWebBrowser;

QDebug operator<<(QDebug debug, const LoadItem &item)
{
    debug.nospace() << "LoadItem:"
        << "\r  Type: " << (int)item.type()
        << "\r  ID: " << item.urlWithId()->idsString()
        << "\r  Url: " << item.urlWithId()->urlsString()
        << "\r  Width: " << item.width()
        << "\r  Height: " << item.height()
        << "\r  Left: " << item.left()
        << "\r  Top: " << item.top()
        << "\r  Right: " << item.right()
        << "\r  Bottom: " << item.bottom()
        << "\r  AllowDialogs: " << item.allowDialogs()
        << "\r  EnableFlash: " << item.enableFlash()
        << "\r  Cookies: " << (int)item.cookies()
        << "\r  EmbedScripts: " << item.embedScripts()
        << "\r  EmbedCode: " << item.embedCode()
        << "\r  Preload: " << item.preload()
        << "\r  UserAgent: " << (int)item.userAgent()
        << "\r  CloseAfter: " << item.closeAfterSeconds()
        << "\r  ShowTime: " << item.showTime()
        << "\r  StartHidden: " << item.startHidden()
        << "\r  AllowWindowOpen: " << item.allowWindowOpen()
        << "\r  UrlFilter: " << item.urlFilter();
    return debug.space();
}

LoadItem::LoadItem()
{
}

LoadItem::LoadItem(BrowserType _type, UrlWithId _uwid,
                                 int _w, int _h, int _l, int _t, int _r, int _b,
                                 bool _allowD, bool _enableF, BrowserCookies _cook, QStringList _embedS, QString _embedC,
                                 bool _preload,
                                 QString _contentT, QString _creativeT, QString _clickU,
                                 BrowserUserAgent _uA, int _cA, int _sT, bool _sH, bool _allowWO, int _group, bool _uF, bool _uIE)
    : mType(_type)
    , mUrlWithId(_uwid)
    , mWidth(_w)
    , mHeight(_h)
    , mLeft(_l)
    , mTop(_t)
    , mRight(_r)
    , mBottom(_b)
    , mAllowDialogs(_allowD)
    , mEnableFlash(_enableF)
    , mCookies(_cook)
    , mEmbedScripts(_embedS)
    , mEmbedCode(_embedC)
    , mPreload(_preload)
    , mContentType(_contentT)
    , mCreativeType(_creativeT)
    , mClickUrl(_clickU)
    , mUserAgent(_uA)
    , mHostUserAgent("")
    , mFixedBottomSpace(0)
    , mFixedFullscreenBottomSpace(0)
    , mEventShownRegistered(false)
    , mEventHideRegistered(false)
    , mEventCompleteRegistered(false)
    , mCloseAfterSeconds(_cA)
    , mShowTime(_sT)
    , mStartHidden(_sH)
    , mAllowWindowOpen(_allowWO)
    , mEngineHttpHost("")
    , mEngineHttpPort(0)
    , mGroupId(_group)
    , mUrlFilter(_uF)
    , mUseIE(_uIE)
{
}

LoadItem::LoadItem(const LoadItem &other)
    : mType(other.type())
    , mUrlWithId(*other.urlWithId())
    , mWidth(other.width())
    , mHeight(other.height())
    , mLeft(other.left())
    , mTop(other.top())
    , mRight(other.right())
    , mBottom(other.bottom())
    , mAllowDialogs(other.allowDialogs())
    , mEnableFlash(other.enableFlash())
    , mCookies(other.cookies())
    , mEmbedScripts(other.embedScripts())
    , mEmbedCode(other.embedCode())
    , mPreload(other.preload())
    , mContentType(other.contentType())
    , mCreativeType(other.creativeType())
    , mClickUrl(other.clickUrl())
    , mUserAgent(other.userAgent())
    , mHostUserAgent(other.hostUserAgent())
    , mFixedBottomSpace(other.fixedBottomSpace())
    , mFixedFullscreenBottomSpace(other.fixedFullscreenBottomSpace())
    , mEventShownRegistered(false)
    , mEventHideRegistered(false)
    , mEventCompleteRegistered(false)
    , mCloseAfterSeconds(other.closeAfterSeconds())
    , mStartHidden(other.startHidden())
    , mShowTime(other.showTime())
    , mAllowWindowOpen(other.allowWindowOpen())
    , mEngineHttpHost(other.engineHttpHost())
    , mEngineHttpPort(other.engineHttpPort())
    , mGroupId(other.groupId())
    , mUrlFilter(other.urlFilter())
    , mUseIE(other.useIE())
{
}

LoadItem::~LoadItem()
{
}

bool LoadItem::operator ==(const LoadItem &other) const
{
    return type()==other.type() && (*urlWithId())==(*other.urlWithId());
}

bool LoadItem::operator !=(const LoadItem &other) const
{
    return type()!=other.type() || (*urlWithId())!=(*other.urlWithId());
}

BrowserType LoadItem::type() const
{
    return mType;
}

const UrlWithId *LoadItem::urlWithId() const
{
    return &mUrlWithId;
}

int LoadItem::width() const
{
    return mWidth;
}

int LoadItem::height() const
{
    return mHeight;
}

int LoadItem::left() const
{
    return mLeft;
}

int LoadItem::top() const
{
    return mTop;
}

int LoadItem::right() const
{
    return mRight;
}

int LoadItem::bottom() const
{
    return mBottom;
}

bool LoadItem::allowDialogs() const
{
    return mAllowDialogs;
}

bool LoadItem::allowWindowOpen() const
{
    return mAllowWindowOpen;
}

bool LoadItem::enableFlash() const
{
    return mEnableFlash;
}

BrowserCookies LoadItem::cookies() const
{
    return mCookies;
}

QStringList LoadItem::embedScripts() const
{
    return mEmbedScripts;
}

QString LoadItem::embedCode() const
{
    return mEmbedCode;
}

bool LoadItem::preload() const
{
    return mPreload;
}

QString LoadItem::contentType() const
{
    return mContentType;
}

QString LoadItem::creativeType() const
{
    return mCreativeType;
}

QString LoadItem::clickUrl() const
{
    return mClickUrl;
}

BrowserUserAgent LoadItem::userAgent() const
{
    return mUserAgent;
}

void LoadItem::setSize(int w, int h)
{
    mWidth = w;
    mHeight = h;
}

void LoadItem::setEngineHttpHost(const QString &host)
{
    mEngineHttpHost = host;
}

void LoadItem::setEngineHttpPort(int port)
{
    mEngineHttpPort = port;
}

QString LoadItem::engineHttpHost() const
{
    return mEngineHttpHost;
}

int LoadItem::engineHttpPort() const
{
    return mEngineHttpPort;
}

int LoadItem::groupId() const
{
    return mGroupId;
}

QString AceWebBrowser::LoadItem::hostUserAgent() const
{
    return mHostUserAgent;
}

unsigned int LoadItem::fixedBottomSpace() const
{
    return mFixedBottomSpace;
}

unsigned int LoadItem::fixedFullscreenBottomSpace() const
{
    return mFixedFullscreenBottomSpace;
}

bool LoadItem::shownRegistered() const
{
    return mEventShownRegistered;
}

bool LoadItem::hideRegistered() const
{
    return mEventHideRegistered;
}

void LoadItem::setHostUserAgent(const QString &ua)
{
    mHostUserAgent = ua;
}

void LoadItem::setFixedBottomSpace(unsigned int space)
{
    mFixedBottomSpace = space;
}

void LoadItem::setFixedFullscreenBottomSpace(unsigned int space)
{
    mFixedFullscreenBottomSpace = space;
}

void LoadItem::setShownRegistered(bool val)
{
    mEventShownRegistered = val;
}

void LoadItem::setHideRegistered(bool val)
{
    mEventHideRegistered = val;
}

void LoadItem::clearEventFlags()
{
    setShownRegistered(false);
    setHideRegistered(false);
}

int LoadItem::closeAfterSeconds() const
{
    return mCloseAfterSeconds;
}

bool LoadItem::startHidden() const
{
    return mStartHidden;
}

int LoadItem::showTime() const
{
    return mShowTime;
}

bool LoadItem::urlFilter() const
{
    return mUrlFilter;
}

bool LoadItem::useIE() const
{
    return mUseIE;
}
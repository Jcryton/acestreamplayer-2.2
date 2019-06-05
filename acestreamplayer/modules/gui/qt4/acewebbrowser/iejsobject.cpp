#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <p2p_object.h>
#include <utility>
#include <fstream>
#include "acewebbrowser/iejsobject.hpp"
#include <cstring>
#include <qaxtypes.h>
#include <QDebug>
#include <QList>
#include <QStringList>

using namespace AceWebBrowser;

JSObject::JSObject()
    : ref(0)
    , mPlayerInFullscreen(false)
    , mFlashEnabled(true)
    , mBrowserIsVisible(false)
    , mHostUserAgent("")
    , mBrowserSize(0,0)
{
    idMap.insert(std::make_pair(L"browserClose", DISPID_USER_BROWSER_CLOSE));
    idMap.insert(std::make_pair(L"browserHide", DISPID_USER_BROWSER_HIDE));
    idMap.insert(std::make_pair(L"browserShow", DISPID_USER_BROWSER_SHOW));
    idMap.insert(std::make_pair(L"browserCloseAfter", DISPID_USER_BROWSER_CLOSE_AFTER));
    idMap.insert(std::make_pair(L"linkOpen", DISPID_USER_LINK_OPEN));
    idMap.insert(std::make_pair(L"playerPlay", DISPID_USER_PLAYER_PLAY));
    idMap.insert(std::make_pair(L"playerPause", DISPID_USER_PLAYER_PAUSE));
    idMap.insert(std::make_pair(L"playerStop", DISPID_USER_PLAYER_STOP));
    idMap.insert(std::make_pair(L"linkOpenLater", DISPID_USER_OPEN_LINK_LATER));
    idMap.insert(std::make_pair(L"fillPlayerSize", DISPID_USER_FILL_PLAYER_SIZE));
    idMap.insert(std::make_pair(L"playerSetFullscreen", DISPID_USER_PLAYER_SET_FULLSCREEN));
    idMap.insert(std::make_pair(L"browserSetSize", DISPID_USER_BROWSER_SET_SIZE));
    idMap.insert(std::make_pair(L"sendEvent", DISPID_USER_SEND_EVENT));

    idMap.insert(std::make_pair(L"playerIsFullscreen", DISPID_USER_PLAYER_IS_FULLSCREEN));
    idMap.insert(std::make_pair(L"browserIsVisible", DISPID_USER_BROWSER_IS_VISIBLE));
    idMap.insert(std::make_pair(L"pluginVersion", DISPID_USER_PLUGIN_VERSION));
    idMap.insert(std::make_pair(L"flashEnabled", DISPID_USER_FLASH_ENABLED));
    idMap.insert(std::make_pair(L"getHostUserAgent", DISPID_USER_HOST_USER_AGENT));
    idMap.insert(std::make_pair(L"playerWidth", DISPID_USER_PLAYER_WIDTH));
    idMap.insert(std::make_pair(L"playerHeight", DISPID_USER_PLAYER_HEIGHT));
    idMap.insert(std::make_pair(L"browserWidth", DISPID_USER_BROWSER_WIDTH));
    idMap.insert(std::make_pair(L"browserHeight", DISPID_USER_BROWSER_HEIGHT));

    idMap.insert(std::make_pair(L"debug", DISPID_USER_DEBUG));

    idMap.insert(std::make_pair(L"playerToggleFullscreen", DISPID_USER_PLAYER_TOGGLE_FULLSCREEN));
}

HRESULT STDMETHODCALLTYPE JSObject::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IUnknown || riid == IID_IDispatch) {
        *ppv = static_cast<IDispatch*>(this);
    }

    if (*ppv != NULL) {
        //qDebug() << "JSObject::QueryInterface: AddRef()";
        AddRef();
        return S_OK;
    }
    //qDebug() << "JSObject::QueryInterface: no interface";

    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE JSObject::AddRef()
{
    //qDebug() << "JSObject::AddRef: ref=" << ref;
    return InterlockedIncrement(&ref);
}

ULONG STDMETHODCALLTYPE JSObject::Release()
{
    int tmp = InterlockedDecrement(&ref);
    //qDebug() << "JSObject::Release: ref=" << tmp;

    if (tmp == 0) {
        qDebug() << "JSObject::Release: delete this";
        delete this;
    }

    return tmp;
}

HRESULT STDMETHODCALLTYPE JSObject::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 0;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE JSObject::GetTypeInfo(UINT iTInfo, LCID lcid,
    ITypeInfo **ppTInfo)
{
    Q_UNUSED(iTInfo)
    Q_UNUSED(lcid)
    Q_UNUSED(ppTInfo)
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE JSObject::GetIDsOfNames(REFIID riid,
    LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
    Q_UNUSED(riid)
    Q_UNUSED(lcid)
    HRESULT hr = S_OK;

    for (UINT i = 0; i < cNames; i++) {
        std::map<std::wstring, DISPID>::iterator iter = idMap.find(rgszNames[i]);
        if (iter != idMap.end()) {
            rgDispId[i] = iter->second;
            //qDebug() << "JSObject::GetIDsOfNames: success";
        } else {
            rgDispId[i] = DISPID_UNKNOWN;
            hr = DISP_E_UNKNOWNNAME;
            //qDebug() << "JSObject::GetIDsOfNames: failed";
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE JSObject::Invoke(DISPID dispIdMember, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult,
    EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    Q_UNUSED(lcid)
    Q_UNUSED(riid)
    Q_UNUSED(pExcepInfo)
    Q_UNUSED(puArgErr)

    if (wFlags & DISPATCH_METHOD) {
        HRESULT hr = S_OK;

        qDebug() << "JSObject::Invoke: dispIdMember" << dispIdMember << " wFlags" << wFlags << "args" << pDispParams->cArgs;

        // reverse list, avoid decrementing below zero (because of uint overflow)
        QList<QVariant> args;
        for (size_t i = pDispParams->cArgs; i > 0 ; --i) {
            QVariant v = VARIANTToQVariant(pDispParams->rgvarg[i-1], 0);
            args.append(v);
        }

        switch (dispIdMember) {
            case DISPID_USER_BROWSER_CLOSE: {
                qDebug() << "JSObject::Invoke: browserClose";
                emit jsoCloseBrowser();
                break;
            }
            case DISPID_USER_BROWSER_HIDE: {
                qDebug() << "JSObject::Invoke: browserHide";
                emit jsoHideBrowser();
                break;
            }
            case DISPID_USER_BROWSER_SHOW: {
                qDebug() << "JSObject::Invoke: browserShow";
                emit jsoShowBrowser();
                break;
            }
            case DISPID_USER_BROWSER_CLOSE_AFTER: {
                unsigned int seconds = args.at(0).toInt();
                qDebug() << "JSObject::Invoke: browserCloseAfter" << seconds;
                emit jsoCloseBrowserAfter(seconds);
                break;
            }
            case DISPID_USER_LINK_OPEN: {
                QString url = args.at(0).toString();
                bool newwindow = args.at(1).toInt();
                bool aceweb = false;
                QString cmdline = "";
                if(args.size() > 2)  {
                    aceweb = args.at(2).toInt();
                }
                if(args.size() > 3)  {
                    cmdline = args.at(3).toString();
                }
                qDebug() << "JSObject::Invoke: linkOpen" << url << newwindow << aceweb << cmdline;
                emit jsoLinkOpen(url, newwindow, aceweb, cmdline);
                break;
            }
            case DISPID_USER_PLAYER_PLAY: {
                qDebug() << "JSObject::Invoke: playerPlay";
                emit jsoPlayerPlay();
                break;
            }
            case DISPID_USER_PLAYER_PAUSE: {
                qDebug() << "JSObject::Invoke: playerPause";
                emit jsoPlayerPause();
                break;
            }
            case DISPID_USER_PLAYER_STOP: {
                qDebug() << "JSObject::Invoke: playerStop";
                emit jsoPlayerStop();
                break;
            }
            case DISPID_USER_OPEN_LINK_LATER: {
                QString url = args.at(0).toString();
                bool priority = args.at(1).toInt();
                bool aceweb = false;
                QString cmdline = "";
                if(args.size() > 2)  {
                    aceweb = args.at(2).toInt();
                }
                if(args.size() > 3)  {
                    cmdline = args.at(3).toString();
                }
                qDebug() << "JSObject::Invoke: linkOpenLater" << url << priority << aceweb << cmdline;
                emit jsoLinkOpenLater(url, priority, aceweb, cmdline);
                break;
            }
            case DISPID_USER_FILL_PLAYER_SIZE: {
                qDebug() << "JSObject::Invoke: fillPlayerSize";
                emit jsoFillPlayerSize();
                break;
            }
            case DISPID_USER_PLAYER_SET_FULLSCREEN: {
                bool fs = args.at(0).toInt();
                qDebug() << "JSObject::Invoke: playerSetFullscreen" << fs;
                emit jsoPlayerSetFullscreen(fs);
                break;
            }
            case DISPID_USER_BROWSER_SET_SIZE: {
                unsigned int w = args.at(0).toInt();
                unsigned int h = args.at(1).toInt();
                qDebug() << "JSObject::Invoke: browserSetSize" << w << h;
                emit jsoBrowserSetSize(QSize(w, h));
                break;
            }
            case DISPID_USER_SEND_EVENT: {
                QString event = args.at(0).toString();
                qDebug() << "JSObject::Invoke: sendEvent:" << event;
                emit jsoSendEvent(event);
                break;
            }
            case DISPID_USER_PLAYER_IS_FULLSCREEN: {
                qDebug() << "JSObject::Invoke: playerIsFullscreen " << mPlayerInFullscreen;
                QVariantToVARIANT(QVariant(mPlayerInFullscreen), *pVarResult);
                break;
            }
            case DISPID_USER_BROWSER_IS_VISIBLE: {
                qDebug() << "JSObject::Invoke: browserIsVisible";
                QVariantToVARIANT(QVariant(mBrowserIsVisible), *pVarResult);
                break;
            }
            case DISPID_USER_PLUGIN_VERSION: {
                qDebug() << "JSObject::Invoke: pluginVersion";
                QVariantToVARIANT(QVariant(QString(""P2P_STD_VERSION"")), *pVarResult);
                break;
            }
            case DISPID_USER_FLASH_ENABLED: {
                qDebug() << "JSObject::Invoke: flashEnabled";
                QVariantToVARIANT(QVariant(mFlashEnabled), *pVarResult);
                break;
            }
            case DISPID_USER_HOST_USER_AGENT: {
                qDebug() << "JSObject::Invoke: getHostUserAgent";
                QVariantToVARIANT(QVariant(mHostUserAgent), *pVarResult);
                break;
            }
            case DISPID_USER_PLAYER_WIDTH: {
                qDebug() << "JSObject::Invoke: playerWidth";
                QVariantToVARIANT(QVariant(mPlayerSize.width()), *pVarResult);
                break;
            }
            case DISPID_USER_PLAYER_HEIGHT: {
                qDebug() << "JSObject::Invoke: playerHeight";
                QVariantToVARIANT(QVariant(mPlayerSize.height()), *pVarResult);
                break;
            }
            case DISPID_USER_BROWSER_WIDTH: {
                qDebug() << "JSObject::Invoke: browserWidth";
                QVariantToVARIANT(QVariant(mBrowserSize.width()), *pVarResult);
                break;
            }
            case DISPID_USER_BROWSER_HEIGHT: {
                qDebug() << "JSObject::Invoke: browserHeight";
                QVariantToVARIANT(QVariant(mBrowserSize.height()), *pVarResult);
                break;
            }
            case DISPID_USER_DEBUG: {
                QString msg = args.at(0).toString();
                qDebug() << "[JavaScriptObject] " << msg;
                break;
            }
            case DISPID_USER_PLAYER_TOGGLE_FULLSCREEN: {
                qDebug() << "JSObject::Invoke: playerToggleFullscreen";
                emit jsoPlayerToggleFullscreen();
                break;
            }
            default: {
                qDebug() << "JSObject::Invoke: member not found";
                hr = DISP_E_MEMBERNOTFOUND;
            }
        }

        return hr;
    }

    return E_FAIL;
}

void JSObject::setIsInFullscreen(bool value)
{
    if(value != mPlayerInFullscreen) {
        mPlayerInFullscreen = value;
    }
}

void JSObject::setFlashEnabled(bool value)
{
    if(value != mFlashEnabled) {
        mFlashEnabled = value;
    }
}

void JSObject::setVisiability(bool value)
{
    if(value != mBrowserIsVisible) {
        mBrowserIsVisible = value;
    }
}

void JSObject::setHostUserAgent(const QString &value)
{
    if(value != mHostUserAgent) {
        mHostUserAgent = value;
    }
}

void JSObject::invokeJSMethod(const QString &raw)
{
    QStringList params = raw.split(";");
    if(params.size() <= 0) {
        return;
    }

    if(!params.at(0).compare("browserClose")) {
        qDebug() << "JSObject::invokeJSMethod: browserClose";
        emit jsoCloseBrowser();
    }
    else if(!params.at(0).compare("browserHide")) {
        qDebug() << "JSObject::invokeJSMethod: browserHide";
        emit jsoHideBrowser();
    }
    else if(!params.at(0).compare("browserShow")) {
        qDebug() << "JSObject::invokeJSMethod: browserShow";
        emit jsoShowBrowser();
    }
    else if(!params.at(0).compare("browserCloseAfter") && params.size() >= 2) {
        bool ok;
        unsigned int seconds = params.at(1).toInt(&ok);
        if(ok) {
            qDebug() << "JSObject::invokeJSMethod: browserCloseAfter" << seconds;
            emit jsoCloseBrowserAfter(seconds);
        }
    }
    else if(!params.at(0).compare("linkOpen") && params.size() >= 3) {
        QString url = params.at(1);
        bool newwindow = !params.at(2).compare("true", Qt::CaseInsensitive);
        bool aceweb = false;
        QString cmdline = "";
        if(params.size() > 3) {
            aceweb = !params.at(3).compare("true", Qt::CaseInsensitive);
        }
        if(params.size() > 4) {
            cmdline = params.at(4);
        }
        qDebug() << "JSObject::invokeJSMethod: linkOpen" << url << newwindow << aceweb << cmdline;
        emit jsoLinkOpen(url, newwindow, aceweb, cmdline);
    }
    else if(!params.at(0).compare("playerPlay")) {
        qDebug() << "JSObject::invokeJSMethod: playerPlay";
        emit jsoPlayerPlay();
    }
    else if(!params.at(0).compare("playerPause")) {
        qDebug() << "JSObject::invokeJSMethod: playerPause";
        emit jsoPlayerPause();
    }
    else if(!params.at(0).compare("playerStop")) {
        qDebug() << "JSObject::invokeJSMethod: playerStop";
        emit jsoPlayerStop();
    }
    else if(!params.at(0).compare("linkOpenLater") && params.size() >= 3) {
        QString url = params.at(1);
        bool priority = !params.at(2).compare("true", Qt::CaseInsensitive);
        bool aceweb = false;
        QString cmdline = "";
        if(params.size() > 3) {
            aceweb = !params.at(3).compare("true", Qt::CaseInsensitive);
        }
        if(params.size() > 4) {
            cmdline = params.at(4);
        }
        qDebug() << "JSObject::invokeJSMethod: linkOpenLater" << url << priority << aceweb << cmdline;
        emit jsoLinkOpenLater(url, priority, aceweb, cmdline);
    }
    else if(!params.at(0).compare("fillPlayerSize")) {
        qDebug() << "JSObject::invokeJSMethod: fillPlayerSize";
        emit jsoFillPlayerSize();
    }
    else if(!params.at(0).compare("playerSetFullscreen") && params.size() >= 2) {
        bool fs = !params.at(1).compare("true", Qt::CaseInsensitive);
        qDebug() << "JSObject::invokeJSMethod: playerSetFullscreen" << fs;
        emit jsoPlayerSetFullscreen(fs);
    }
    else if(!params.at(0).compare("browserSetSize") && params.size() >= 3) {
        bool ok0, ok1;
        unsigned int w = params.at(1).toInt(&ok0);
        unsigned int h = params.at(2).toInt(&ok1);
        if(ok0 && ok1) {
            qDebug() << "JSObject::invokeJSMethod: browserSetSize" << w << h;
            emit jsoBrowserSetSize(QSize(w, h));
        }
    }
    else if(!params.at(0).compare("sendEvent") && params.size() >= 2) {
        QString event = params.at(1);
        qDebug() << "JSObject::invokeJSMethod: sendEvent:" << event;
        emit jsoSendEvent(event);
    }
    else if(!params.at(0).compare("debug") && params.size() >= 2) {
        qDebug() << "[JavaScriptObject] " << params.at(1);
    }
    else if(!params.at(0).compare("playerToggleFullscreen")) {
        qDebug() << "JSObject::invokeJSMethod: playerToggleFullscreen";
        emit jsoPlayerToggleFullscreen();
    }
    else {
        qDebug() << "JSObject::invokeJSMethod: unknown";
    }
}

void JSObject::handlePlayerSizeChanged(const QSize &size)
{
    mPlayerSize = size;
}

void JSObject::handleBrowserSizeChanged(const QSize &size)
{
    mBrowserSize = size;
}


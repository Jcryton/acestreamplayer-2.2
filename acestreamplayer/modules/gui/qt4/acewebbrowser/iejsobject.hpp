#ifndef IEJSOBJECT_H
#define IEJSOBJECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include <QSize>

#include <ole2.h>
#include <windows.h>
#include <string>
#include <map>

#define JSO_NAME "AceStreamBrowser"

namespace AceWebBrowser {

class JSObject : public QObject, public IDispatch {
    Q_OBJECT
private:
    static const DISPID DISPID_USER_BROWSER_CLOSE = DISPID_VALUE + 1;
    static const DISPID DISPID_USER_BROWSER_HIDE = DISPID_VALUE + 2;
    static const DISPID DISPID_USER_BROWSER_SHOW = DISPID_VALUE + 3;
    static const DISPID DISPID_USER_BROWSER_CLOSE_AFTER = DISPID_VALUE + 4;
    static const DISPID DISPID_USER_LINK_OPEN = DISPID_VALUE + 5;
    static const DISPID DISPID_USER_PLAYER_PLAY = DISPID_VALUE + 6;
    static const DISPID DISPID_USER_PLAYER_PAUSE = DISPID_VALUE + 7;
    static const DISPID DISPID_USER_PLAYER_STOP = DISPID_VALUE + 8;
    static const DISPID DISPID_USER_OPEN_LINK_LATER = DISPID_VALUE + 9;
    static const DISPID DISPID_USER_FILL_PLAYER_SIZE = DISPID_VALUE + 10;
    static const DISPID DISPID_USER_PLAYER_SET_FULLSCREEN = DISPID_VALUE + 11;
    static const DISPID DISPID_USER_BROWSER_SET_SIZE = DISPID_VALUE + 12;
    static const DISPID DISPID_USER_SEND_EVENT = DISPID_VALUE + 13;

    static const DISPID DISPID_USER_PLAYER_IS_FULLSCREEN = DISPID_VALUE + 14;
    static const DISPID DISPID_USER_BROWSER_IS_VISIBLE = DISPID_VALUE + 15;
    static const DISPID DISPID_USER_PLUGIN_VERSION = DISPID_VALUE + 16;
    static const DISPID DISPID_USER_FLASH_ENABLED = DISPID_VALUE + 17;
    static const DISPID DISPID_USER_HOST_USER_AGENT = DISPID_VALUE + 18;
    static const DISPID DISPID_USER_PLAYER_WIDTH = DISPID_VALUE + 19;
    static const DISPID DISPID_USER_PLAYER_HEIGHT = DISPID_VALUE + 20;
    static const DISPID DISPID_USER_BROWSER_WIDTH = DISPID_VALUE + 21;
    static const DISPID DISPID_USER_BROWSER_HEIGHT = DISPID_VALUE + 22;

    static const DISPID DISPID_USER_DEBUG = DISPID_VALUE + 23;
    static const DISPID DISPID_USER_PLAYER_TOGGLE_FULLSCREEN = DISPID_VALUE + 24;

    std::map<std::wstring, DISPID> idMap;
    long ref;

    std::map<std::string, std::string> values;

public:
    JSObject();

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDispatch
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid,
        ITypeInfo **ppTInfo);
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid,
        LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
    virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult,
        EXCEPINFO *pExcepInfo, UINT *puArgErr);

    void setIsInFullscreen(bool value);
    void setFlashEnabled(bool value);
    void setVisiability(bool value);
    void setHostUserAgent(const QString &value);
    void invokeJSMethod(const QString &raw);

private:
    bool mPlayerInFullscreen;
    bool mFlashEnabled;
    bool mBrowserIsVisible;
    QString mHostUserAgent;
    QSize mBrowserSize;
    QSize mPlayerSize;

signals:
    void jsoCloseBrowser();
    void jsoHideBrowser();
    void jsoShowBrowser();
    void jsoCloseBrowserAfter(unsigned int);
    void jsoLinkOpen(QString url, bool openInNewWindow, bool openInAceWeb, QString arguments);
    void jsoPlayerPlay();
    void jsoPlayerPause();
    void jsoPlayerStop();
    void jsoLinkOpenLater(QString url, bool high_priority, bool openInAceWeb, QString arguments);
    void jsoFillPlayerSize();
    void jsoPlayerSetFullscreen(bool);
    void jsoBrowserSetSize(const QSize&);
    void jsoSendEvent(QString);
    void jsoPlayerToggleFullscreen();

public slots:
    void handlePlayerSizeChanged(const QSize &);
    void handleBrowserSizeChanged(const QSize &);
};

}

#endif // IEJSOBJECT_H

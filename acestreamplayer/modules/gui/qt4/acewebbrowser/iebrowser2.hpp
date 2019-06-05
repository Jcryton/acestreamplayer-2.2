#ifndef IEBROWSER2_H
#define IEBROWSER2_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QWidget>

#include <windows.h>
#include "acewebbrowser/loaditem.hpp"
#include "acewebbrowser/iewebview.hpp"
#include "acewebbrowser/iejsobject.hpp"
#include "acewebbrowser/javascriptobject.hpp"

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace AceWebBrowser {

class IEBrowser2 : public QWidget
{
    Q_OBJECT
public:
    IEBrowser2(const LoadItem &item, QWidget *parent = 0);
    ~IEBrowser2();

    QString baseUrl() const;
    BrowserType type();
    QStringList embedScripts();
    QString embedCode();
    
    void setShowAvailable(bool);
    bool showAvailable() const;
    void setVisiabilityProcessingEnable(bool);

    JSObject *javaScriptObject();

    void load(const LoadItem &item);

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void updateSizing(const QSize &size);
    void updateSizing();
    void updatePosition();

    void registryCheck();

private:
    IEWebView *mWebView;
    JSObject *mJSO;
    LoadItem mItem;
    BrowserState mState;
    bool mIsAd;

    int mParentState;
    bool mParentFullscreen;
    
    bool mShowAvailable;
    bool mVisiabilityProcessingEnable;

    QTimer *mDeferredTimer;

signals:
    void notifyBrowserVisiabilityChanged(AceWebBrowser::BrowserType, bool);
    void notifyBrowserSizeChanged(const QSize &);
    void notifyNeedExitFullscreen();
    void notifyParentCommandToShow(AceWebBrowser::BrowserType type);

    void registerBrowserSuccessEvent(AceWebBrowser::BrowserType, QString);
    void registerBrowserErrorEvent(AceWebBrowser::BrowserType, QString);
    void registerBrowserSendEvent(AceWebBrowser::BrowserType type, QString event_name, QString id);

private slots:
    // actions
    void showBrowser();
    void hideBrowser();
    void closeBrowser(bool failed = false);
    void deferredCloseBrowser(unsigned int);

    // webview
    void ieProgressChange(int,int);
    void ieDocumentComplete(IDispatch*,QVariant&);
    void ieDownloadBegin();
    void ieDownloadComplete();
    void ieNavigateError(IDispatch*,QVariant&,QVariant&,QVariant&,bool&);
    void ieException(int, const QString&, const QString&, const QString&);
    void ieBeforeNavigate2(IDispatch*,QVariant&,QVariant&,QVariant&,QVariant&,QVariant&,bool&);
    void ieWindowClosing(bool, bool&);
    void ieWindowOpen(IDispatch**,bool&,uint,QString,QString);

    // jso
    void handleJSOFillParentSizeCommand();
    void handleJSOResizeCommand(QSize);
    void handleJSOSendEvent(QString);

    void openUrl(QString url, bool inNewWindow, bool _openInAceWeb = false, QString arguments = "");

public slots:
    void handleParentSize(const QSize &size);
    void handleParentFullscreen(bool isFullscreen);
    void handleParentState(int state, bool isAd);
};

}

#endif // IEBROWSER2_H

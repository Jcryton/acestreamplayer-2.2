#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/iebrowser2.hpp"
#include "acewebbrowser/iewebview.hpp"
#include "acewebbrowser/exceptions.hpp"

#include <QLayout>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QResizeEvent>
#include <QDesktopServices>
#include <QCoreApplication>

using namespace AceWebBrowser;

IEBrowser2::IEBrowser2(const LoadItem &item, QWidget *parent) :
    QWidget(parent)
  , mItem(item)
  , mState(BS_UNLOADED)
  , mParentState(BHPS_NONE)
  , mParentFullscreen(false)
  , mDeferredTimer(0)
  , mVisiabilityProcessingEnable(true)
  , mShowAvailable(true)
{
    setObjectName("IEBrowser2");

    registryCheck();

    QPalette p = palette();
    p.setColor(QPalette::Window, QColor("#404040"));
    p.setColor(QPalette::WindowText, QColor("#000000"));
    setPalette(p);

    //setWindowFlags(Qt::WindowStaysOnTopHint);

    QLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    try {
        mWebView = new IEWebView(this);
    }
    catch(Exception *e) {
        throw new BrowserException(e->message(), mItem.type(), mItem.urlWithId()->id());
    }

    layout->addWidget(mWebView);
    setLayout(layout);

    connect(mWebView, SIGNAL(ProgressChange(int,int)), SLOT(ieProgressChange(int,int)));
    connect(mWebView, SIGNAL(DocumentComplete(IDispatch*,QVariant&)), SLOT(ieDocumentComplete(IDispatch*,QVariant&)));
    connect(mWebView, SIGNAL(DownloadBegin()), SLOT(ieDownloadBegin()));
    connect(mWebView, SIGNAL(DownloadComplete()), SLOT(ieDownloadComplete()));
    connect(mWebView, SIGNAL(NavigateError(IDispatch*,QVariant&,QVariant&,QVariant&,bool&)),
            SLOT(ieNavigateError(IDispatch*,QVariant&,QVariant&,QVariant&,bool&)));
    connect(mWebView, SIGNAL(BeforeNavigate2(IDispatch*,QVariant&,QVariant&,QVariant&,QVariant&,QVariant&,bool&)),
            SLOT(ieBeforeNavigate2(IDispatch*,QVariant&,QVariant&,QVariant&,QVariant&,QVariant&,bool&)));
    connect(mWebView, SIGNAL(exception(int, const QString&, const QString&, const QString&)),
            SLOT(ieException(int, const QString&, const QString&, const QString&)));
    connect(mWebView, SIGNAL(WindowClosing(bool,bool&)), SLOT(ieWindowClosing(bool,bool&)));
    connect(mWebView, SIGNAL(NewWindow3(IDispatch**,bool&,uint,QString,QString)),
            SLOT(ieWindowOpen(IDispatch**,bool&,uint,QString,QString)));

    mJSO = new JSObject();
    mJSO->AddRef();
    connect(this, SIGNAL(notifyBrowserSizeChanged(QSize)), mJSO, SLOT(handleBrowserSizeChanged(QSize)));
    connect(mJSO, SIGNAL(jsoLinkOpen(QString ,bool, bool, QString)), SLOT(openUrl(QString, bool, bool, QString)));
    connect(mJSO, SIGNAL(jsoCloseBrowser()), SLOT(closeBrowser()));
    connect(mJSO, SIGNAL(jsoShowBrowser()), SLOT(showBrowser()));
    connect(mJSO, SIGNAL(jsoHideBrowser()), SLOT(hideBrowser()));
    connect(mJSO, SIGNAL(jsoCloseBrowserAfter(unsigned int)), SLOT(deferredCloseBrowser(unsigned int)));
    connect(mJSO, SIGNAL(jsoFillPlayerSize()), SLOT(handleJSOFillParentSizeCommand()));
    connect(mJSO, SIGNAL(jsoBrowserSetSize(QSize)), SLOT(handleJSOResizeCommand(QSize)));
    connect(mJSO, SIGNAL(jsoSendEvent(QString)), SLOT(handleJSOSendEvent(QString)));
    mJSO->handlePlayerSizeChanged(parentWidget()->size());

    updateSizing();
    setVisible(false);
}

IEBrowser2::~IEBrowser2()
{
    if(mDeferredTimer) {
        mDeferredTimer->stop();
        delete mDeferredTimer;
    }
    if(mJSO) {
        mJSO->Release();
    }
    qDebug() << "IEBrowser2::~IEBrowser2";
}

QString IEBrowser2::baseUrl() const
{
    return mItem.urlWithId()->url();
}

BrowserType IEBrowser2::type()
{
    return mItem.type();
}

QStringList IEBrowser2::embedScripts()
{
    return mItem.embedScripts();
}

QString IEBrowser2::embedCode()
{
    return mItem.embedCode();
}

JSObject *IEBrowser2::javaScriptObject()
{
    return mJSO;
}

void IEBrowser2::setShowAvailable(bool available)
{
    qDebug() << "IEBrowser2::setShowAvailable: show available" << available << "type" << type();
    mShowAvailable = available;
}

void IEBrowser2::setVisiabilityProcessingEnable(bool enable) 
{
    qDebug() << "IEBrowser2::setVisiabilityProcessingEnable: set enable" << enable << "type" << type();
    mVisiabilityProcessingEnable = enable;
}

bool IEBrowser2::showAvailable() const
{
    return mShowAvailable;
}

void IEBrowser2::load(const LoadItem &item)
{
    if(mWebView) {
        mItem = item;
        updateSizing();
        mJSO->setHostUserAgent(mItem.hostUserAgent());
        mJSO->setFlashEnabled(mItem.enableFlash());
        mWebView->load(QUrl(mItem.urlWithId()->url()));
        mState = BS_LOADING;
    }
}

void IEBrowser2::showEvent(QShowEvent *event)
{
    if(mVisiabilityProcessingEnable) {
        emit notifyBrowserVisiabilityChanged(mItem.type(), true);
        mJSO->setVisiability(true);
    }
    event->accept();
}

void IEBrowser2::hideEvent(QHideEvent *event)
{
    if(mVisiabilityProcessingEnable) {
        emit notifyBrowserVisiabilityChanged(mItem.type(), false);
        mJSO->setVisiability(false);
    }
    event->accept();
}

void IEBrowser2::resizeEvent(QResizeEvent *event)
{
    emit notifyBrowserSizeChanged(event->size());
    event->accept();
}

/******************************
 *     Player handlers
 ******************************/
void IEBrowser2::handleParentSize(const QSize &size)
{
    updateSizing(size);
}

void IEBrowser2::handleParentFullscreen(bool isFullscreen)
{
    if(isFullscreen != mParentFullscreen)
    {
        mParentFullscreen = isFullscreen;
        mJSO->setIsInFullscreen(mParentFullscreen);
    }
}

void IEBrowser2::handleParentState(int state, bool isAd)
{
    if(state != mParentState)
    {
        mParentState = state;
        mIsAd = isAd;
    }
}

/******************************
 *     Positioning
 ******************************/
void IEBrowser2::updateSizing()
{
    updateSizing(QSize(parentWidget()->width(), parentWidget()->height()));
}

void IEBrowser2::updateSizing(const QSize &size)
{
    int pWidth = size.width();
    int pHeight = size.height();

    if(mItem.width() > 0 && mItem.width() < pWidth) {
        setMinimumWidth(mItem.width());
        setMaximumWidth(mItem.width());
    }
    else {
        setMinimumWidth(pWidth);
        setMaximumWidth(pWidth);
    }

    int bottomSpace = (mItem.type() == BTYPE_PREROLL || mItem.type() == BTYPE_PREPLAY)
            ? 0
            : mParentFullscreen
              ? mItem.fixedFullscreenBottomSpace()
              : mItem.fixedBottomSpace();
    if(mItem.height() > 0 && mItem.height() < pHeight - bottomSpace) {
        setMinimumHeight(mItem.height());
        setMaximumHeight(mItem.height());
    }
    else {
        setMinimumHeight(pHeight - bottomSpace);
        setMaximumHeight(pHeight - bottomSpace);
    }

    qDebug() << "IEBrowser2::updateSizing: Parent" << size << "Browser" << minimumSize();
    updatePosition();
}

void IEBrowser2::updatePosition()
{
    int x = -1; int y = -1;
    int bottomSpace = mParentFullscreen ? mItem.fixedFullscreenBottomSpace() : mItem.fixedBottomSpace();

    int pWidth = parentWidget()->width();
    if(mItem.left() >= 0) {
        x = mItem.left() >= pWidth ? 0 : mItem.left();
    }
    else if(mItem.right() >= 0) {
        int tmp_x = pWidth - maximumWidth() - mItem.right();
        x = tmp_x < 0 ? 0 : tmp_x;
    }

    int pHeight = parentWidget()->height();
    if(mItem.top() >= 0) {
            y = mItem.top() >= pHeight - bottomSpace ? 0 : mItem.top();
    }
    else if(mItem.bottom() >= 0) {
        int tmp_y = pHeight - maximumHeight() - mItem.bottom() - bottomSpace;
        y = tmp_y < 0 ? 0 : tmp_y;
    }

    if(x == -1) {
        x = (pWidth > maximumWidth())
                ? (pWidth - maximumWidth()) / 2
                : 0;
    }
    if(y == -1) {
        y = (maximumHeight() + bottomSpace >= pHeight)
                ? (pHeight - maximumHeight() - bottomSpace) / 2
                : ((pHeight > maximumHeight()) ? (pHeight - maximumHeight()) / 2 : 0);
    }

    move(x, y);
}

void IEBrowser2::registryCheck()
{
    QString processName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION", QSettings::NativeFormat);
    if(reg.value(processName, 0).toInt() != 20000) {
        reg.setValue(processName, 20000);
    }
}

/******************************
 *     Private slots
 ******************************/
void IEBrowser2::showBrowser()
{
    if(!mShowAvailable) {
        qDebug() << "IEBrowser2::showBrowser: showing disabled" << type();
        return;
    }

    qDebug() << "IEBrowser2::showBrowser";
    if(mState == BS_LOADED) {
        emit notifyParentCommandToShow(type());
        show();
        raise();
    }
}

void IEBrowser2::hideBrowser()
{
    qDebug() << "IEBrowser2::hideBrowser";
    hide();
}

void IEBrowser2::closeBrowser(bool failed)
{
    qDebug() << "IEBrowser2::closeBrowser";
    hideBrowser();
    mWebView->clear();
    if(failed) {
        emit registerBrowserErrorEvent(mItem.type(), mItem.urlWithId()->id());
    }
    else {
        emit registerBrowserSuccessEvent(mItem.type(), mItem.urlWithId()->id());
    }
}

void IEBrowser2::deferredCloseBrowser(unsigned int seconds)
{
    if(!mDeferredTimer) {
        mDeferredTimer = new QTimer(this);
        mDeferredTimer->setSingleShot(true);
        connect(mDeferredTimer, SIGNAL(timeout()), SLOT(closeBrowser()));
    }
    mDeferredTimer->setInterval(seconds * 1000);
    mDeferredTimer->start();
    hideBrowser();
}

void IEBrowser2::ieProgressChange(int cur, int max)
{
    qDebug() << "IEBrowser2::ieProgressChange:" << cur << "/" << max;
}

void IEBrowser2::ieDocumentComplete(IDispatch *dispatch, QVariant &url)
{
    QUrl qurl(url.toString());
    qDebug() << "IEBrowser2::ieDocumentComplete: complete: url =" << url.toString() << "scheme =" << qurl.scheme();

    if(qurl.scheme() != "http" && qurl.scheme() != "https") {
        return;
    }

    //mWebView->AddCustomObject(dispatch, mJSO, JSO_NAME);
    mWebView->connectEmbedElements(dispatch);
    if(url == QUrl(mItem.urlWithId()->url()) && mState == BS_LOADING) {
        mState = BS_LOADED;
        if(!mItem.startHidden()) {
            showBrowser();
        }
    }
}

void IEBrowser2::ieDownloadBegin()
{
    qDebug() << "IEBrowser2::ieDownloadBegin";
    //if(mState != BS_ERROR)
    //    mState = BS_LOADING;
}

void IEBrowser2::ieDownloadComplete()
{
    qDebug() << "IEBrowser2::ieDownloadComplete";
}

void IEBrowser2::ieNavigateError(IDispatch *, QVariant &url, QVariant &targetFrameName, QVariant &statusCode, bool &cancel)
{
    qDebug() << "IEBrowser2::ieNavigateError: url =" << url.toString() << "frame =" << targetFrameName.toString() << "code =" << statusCode.toString();
    if(url == QUrl(mItem.urlWithId()->url())) {
        cancel = true;
        mState = BS_ERROR;
        closeBrowser(true);
    }
}

void IEBrowser2::ieBeforeNavigate2(IDispatch *frame, QVariant &url, QVariant &flags, QVariant &targetFrameName, QVariant &postData, QVariant &headers, bool &cancel)
{
    Q_UNUSED(frame)
    Q_UNUSED(flags)
    Q_UNUSED(postData)
    Q_UNUSED(headers)
    Q_UNUSED(cancel)

    // detect whether we are in frame or not
    bool isFrame = false;
    IWebBrowser2 *wb;
    IDispatch *container;

    if(SUCCEEDED(frame->QueryInterface(IID_IWebBrowser2, (void**)&wb)) && wb) {
        if(SUCCEEDED(wb->get_Parent(&container)) && container) {
            isFrame = true;
            container->Release();
        }
        wb->Release();
    }
    
    QUrl qurl(url.toString());
    qDebug() << "IEBrowser2::ieBeforeNavigate2: url =" << url.toString() << "hash =" << qurl.fragment() << "frame =" << targetFrameName.toString();
    if(qurl.scheme() != "http" && qurl.scheme() != "https") {
        return;
    }

    QString fragment = qurl.fragment();
    if(fragment.startsWith("acestream:")) {
        fragment = fragment.right(fragment.size() - 10);
        mJSO->invokeJSMethod(fragment);
        
        // prevent browser from actual navigating
        cancel = true;
    }
    
    if(!isFrame) {
        // top level
        if(qurl.host() != QUrl(mItem.urlWithId()->url()).host()) {
            qDebug() << "IEBrowser2::ieBeforeNavigate2: cancel navigation: host =" << url.toString() << "dest =" << mItem.urlWithId()->url();
            cancel = true;
            openUrl(qurl.toString(), true);
        }
    }
}

void IEBrowser2::ieException(int code, const QString& source, const QString& desc, const QString& help)
{
    Q_UNUSED(help)
    qDebug() << "IEBrowser2::ieException: code =" << code << "source =" << source << "desc =" << desc;
}

void IEBrowser2::ieWindowClosing(bool isChildWindow, bool &cancel)
{
    Q_UNUSED(isChildWindow)

    qDebug() << "IEBrowser2::ieWindowClosing: isChildWindow =" << isChildWindow;
    cancel = true;
}

void IEBrowser2::ieWindowOpen(IDispatch **, bool &cancel, uint, QString context, QString url)
{
    if(!mItem.allowWindowOpen()) {
        qDebug() << "IEBrowser2::ieWindowOpen: creating new windows disabled";
    }
    else {
        qDebug() << "IEBrowser2::ieWindowOpen: context =" << context << "url =" << url;
        QDesktopServices::openUrl(url);
    }
    cancel = true;
}

void IEBrowser2::handleJSOFillParentSizeCommand()
{
    mItem.setFixedBottomSpace(0);
    mItem.setFixedFullscreenBottomSpace(0);
    mItem.setSize(0,0);
    updateSizing();
}

void IEBrowser2::handleJSOResizeCommand(QSize newSz)
{
    mItem.setSize(newSz.width(), newSz.height());
    updateSizing();
}

void IEBrowser2::handleJSOSendEvent(QString event_name)
{
    qDebug() << "IEBrowser2::handleJSOSendEvent: event_name =" << event_name;
    emit registerBrowserSendEvent(mItem.type(), event_name, mItem.urlWithId()->id());
}

void IEBrowser2::openUrl(QString url, bool inNewWindow, bool _openInAceWeb, QString arguments)
{
    qDebug() << "IEBrowser2::openUrl" << url << "new window" << inNewWindow;

    
    if(_openInAceWeb) {
        OpenInAceWeb(QUrl(url), arguments);
        emit notifyNeedExitFullscreen();
    }
    else if(inNewWindow) {
        QDesktopServices::openUrl(url);
        emit notifyNeedExitFullscreen();
    }
    else {
        if(mWebView) {
            // navigate
            mWebView->load(QUrl(url));
        }
    }
}

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/browser.hpp"
#include "acewebbrowser/webpage.hpp"
#include "acewebbrowser/webview.hpp"
#include "acewebbrowser/javascriptobject.hpp"
#include "acewebbrowser/networkmanager.hpp"
#include "acewebbrowser/browsermanager.hpp"

#include <QAction>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QPushButton>
#include <QWebHistory>
#include <QWebFrame>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QTimer>

using namespace AceWebBrowser;

// conditions
bool PauseBrowserAvailable(const Browser *browser) {
    bool ret = false;
    if(browser->manager() && browser->manager()->hasBrowser(AceWebBrowser::BTYPE_PAUSE)) {
        Browser *pause = browser->manager()->getBrowser(AceWebBrowser::BTYPE_PAUSE);
        ret = pause && pause->showAvailable();
    }
    return ret;
}

bool CanShowOnPageLoadingFinished(const Browser *browser) {
    return !browser->showOnlyOnPlayerStateChagned();
}

/*
bool OverlayNotInBrowserMode(const Browser *browser) {
    bool ret = true;
    if(browser->manager() && browser->manager()->hasBrowser(AceWebBrowser::BTYPE_OVERLAY)) {
        Browser *overlay = browser->manager()->getBrowser(AceWebBrowser::BTYPE_OVERLAY);
        ret = overlay && !overlay->isBrowserModeEnabled();
    }
    return ret;
}
*/

Browser::Browser(const LoadItem &item, BrowserManager *manager, QWidget *parent) :
    QWidget(parent)
  , mWebView(0)
  , mJSO(new JavaScriptObject(this))
  , mManager(manager)
  , mItem(item)
  , mNavigationBar(0)
  , mBackAction(0)
  , mFwdAction(0)
  , mCloseAction(0)
  , mIsContentRequested(false)
  , mState(BS_UNLOADED)
  , mParentState(AceWebBrowser::BHPS_NONE)
  , mParentFullscreen(false)
  , mDieing(false)
  , mBrowserModeEnabled(false)
  , mShowOnPlayerStateChangedOnly(false)
  , mShowAvailable(true)
  , mVisiabilityProcessingEnable(true)
  , mDeferredTimer(0)
  , mCloseAfterIntHiddenTimer(0)
  , mHideIntHiddenTimer(0)
  , mNeedsReshowing(false)
{
    setObjectName("Browser");

    QPalette p = palette();
    p.setColor(QPalette::Window, QColor("#404040"));
    p.setColor(QPalette::WindowText, QColor("#000000"));
    setPalette(p);

    //setWindowFlags(Qt::WindowStaysOnTopHint);

    mLayout = new QVBoxLayout(this);
    mLayout->setSpacing(0);
    mLayout->setMargin(0);

    createNavigationBar();
    mLayout->addWidget(mNavigationBar);
    setLayout(mLayout);

    createWebView();
    configureWebPage();

    connect(this, SIGNAL(notifyBrowserSizeChanged(QSize)), mJSO, SLOT(handleBrowserSizeChanged(QSize)));
    connect(mJSO, SIGNAL(jsoLinkOpen(QString ,bool, bool ,QString)), SLOT(openUrl(QString, bool, bool, QString)));
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

Browser::~Browser()
{
    if(mCloseAfterIntHiddenTimer) {
        mCloseAfterIntHiddenTimer->stop();
        delete mCloseAfterIntHiddenTimer;
    }
    if(mHideIntHiddenTimer) {
        mHideIntHiddenTimer->stop();
        delete mHideIntHiddenTimer;
    }
    if(mDeferredTimer) {
        mDeferredTimer->stop();
        delete mDeferredTimer;
    }
}

void Browser::setPlayerFullscreen(bool state)
{
    handleParentFullscreen(state);
}

void Browser::setPlayerState(int state, bool isAd)
{
    handleParentState(state, isAd);
}

QString Browser::baseUrl() const
{
    return mItem.urlWithId()->url();
}

BrowserType Browser::type()
{
    return mItem.type();
}

QString Browser::id()
{
    return mItem.urlWithId()->id();
}

bool Browser::allowWindowOpen()
{
    return mItem.allowWindowOpen();
}

QString Browser::engineHost()
{
    return mItem.engineHttpHost();
}

int Browser::enginePort()
{
    return mItem.engineHttpPort();
}

int Browser::groupId() const
{
    return mItem.groupId();
}

bool Browser::urlFilter() const
{
    return mItem.urlFilter();
}

BrowserCookies Browser::cookiesType()
{
    return mItem.cookies();
}

QStringList Browser::embedScripts()
{
    return mItem.embedScripts();
}

QString Browser::embedCode()
{
    return mItem.embedCode();
}

void Browser::setShowAvailable(bool available)
{
    qDebug() << "Browser::setShowAvailable: show available" << available << "type" << type();
    mShowAvailable = available;
}

void Browser::setVisiabilityProcessingEnable(bool enable) 
{
    qDebug() << "Browser::setVisiabilityProcessingEnable: set enable" << enable << "type" << type();
    mVisiabilityProcessingEnable = enable;
}

bool Browser::showAvailable() const
{
    return mShowAvailable;
}
bool Browser::dieing() const
{
    return mDieing;
}

bool Browser::isBrowserModeEnabled() const
{
    return mBrowserModeEnabled;
}

bool Browser::isFullSized()
{
    return mItem.width() == 0 && mItem.height() == 0 && (mItem.fixedBottomSpace() == 0 || mItem.fixedFullscreenBottomSpace() == 0);
}

JavaScriptObject *Browser::javaScriptObject()
{
    return mJSO;
}

BrowserManager *Browser::manager() const
{
    return mManager;
}

bool Browser::showOnlyOnPlayerStateChagned() const
{
    return mShowOnPlayerStateChangedOnly;
}

bool Browser::isAlreadyLoading(const LoadItem &item)
{
    return mItem == item && item.type() != BTYPE_WEBSTAT;
}

void Browser::load(const LoadItem &item)
{
    if(dieing()) {
        qDebug() << "Browser::load: dieing, skip load: url =" << item.urlWithId()->url();
        return;
    }

    if(mBrowserModeEnabled) {
        qDebug() << "Browser::load: ignored because of user browser mode";
        return;
    }

    // preload independent types
    if(item.type() == BTYPE_PREROLL
            || item.type() == BTYPE_PREPLAY
            || item.type() == BTYPE_WEBSTAT) {
        mItem = item;
        load();
        return;
    }

    if(!isVisible() && !item.preload()) {
        mItem = item;
        mState = BS_UNLOADED;
        return;
    }

    if(mWebView) {
        if(isAlreadyLoading(item) && mIsContentRequested) {
            qDebug() << "Browser::load: Already loaded";
            mItem.clearEventFlags();

            updateSizing();
            return;
        }
        mItem = item;

        load();
    }
}

void Browser::activateBrowserMode(bool activate)
{
    mBrowserModeEnabled = activate;
    if(activate) {
        mItem.setSize(0, 0);
        updateSizing();

        if(mNavigationBar) mNavigationBar->show();
    }
    else {
        if(mNavigationBar) mNavigationBar->hide();
    }
}

void Browser::handleWindowOpen()
{
    activateBrowserMode(true);
}

void Browser::handleWindowClose()
{
    qDebug() << "Browser::handleWindowClose";
    closeBrowser();
}

void Browser::pageLoadStarted()
{
    /*if(mIsContentRequested && mState == BS_LOADED) {
        mIsContentRequested = false;
    }*/
    mState = BS_LOADING;
}

void Browser::pageLoadFinished(bool status)
{
    qDebug() << "Browser::pageLoadFinished: " << mWebView->url() << "loaded" << status << "requested" << mIsContentRequested;

    mState = status ? BS_LOADED : BS_UNLOADED;
    if(status) {
        processPlayerActiveStatesAfterLoading();
    }
    else {
        if(mItem.urlWithId()->next()) {
            qDebug() << "Browser::pageLoadFinished: loading next priopity url";
            load();
        }
        else {
            if(!mBrowserModeEnabled) {
                emit registerBrowserErrorEvent(mItem.type(), mItem.urlWithId()->id());
                closeBrowser(true);
            }
        }
    }
}

void Browser::openUrl(const QUrl &url)
{
    qDebug() << "Browser::openUrl: " << url;

    if(mWebView->linkDelegationPolicy() == QWebPage::DelegateAllLinks) {
        qDebug() << "Browser::openUrl: open as delegated link";
        QDesktopServices::openUrl(url);
        emit notifyNeedExitFullscreen();
    }
    else {
        qDebug() << "Browser::openUrl: open not delegated link";
        mWebView->changeReferer(WebView::RT_CURRENT);
        mWebView->load(url);
        mWebView->setScrollbarsEnable(true);
        activateBrowserMode();
    }
}

void Browser::openUrl(QString url, bool inNewWindow, bool _openInAceWeb, QString arguments)
{
    qDebug() << "Browser::openUrl2: " << url;

    if(_openInAceWeb) {
        OpenInAceWeb(QUrl(url), arguments);
        emit notifyNeedExitFullscreen();
    }
    else if(inNewWindow) {
        QDesktopServices::openUrl(QUrl(url));
        emit notifyNeedExitFullscreen();
    }
    else {
        openUrl(QUrl(url));
    }
}

void Browser::createWebView()
{
    if(!mWebView) {
        mWebView = new WebView(this);
        mLayout->addWidget(mWebView);

        mWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, mItem.enableFlash());

        mWebView->setJavaScriptWindowObject(mJSO);

        connect(mWebView->page()->mainFrame(), SIGNAL(loadStarted()), SLOT(pageLoadStarted()));
        connect(mWebView->page(), SIGNAL(loadFinished(bool)), SLOT(updateActionButtons()));
        connect(mWebView, SIGNAL(pageNetworkFinished(bool)), SLOT(pageLoadFinished(bool)));
        connect(mWebView, SIGNAL(linkClicked(const QUrl&)), SLOT(openUrl(const QUrl &)));
        connect(mWebView->page(), SIGNAL(windowCloseRequested()), SLOT(handleWindowClose()));

        connect(mWebView, SIGNAL(windowOpenedInSameBrowser()), SLOT(handleWindowOpen()));
        connect(mWebView, SIGNAL(gotFocus()), SLOT(handleGotFocus()));
    }
}

void Browser::handleGotFocus()
{
    qDebug() << "Browser::handleGotFocus: visible" << isVisible();
    if(isVisible() && !isFullSized()) {
        emit gotFocus();
    }
}

void Browser::deleteWebView()
{
    if(mWebView) {
        mLayout->removeWidget(mWebView);
        mWebView->deleteLater();
    }
}

void Browser::configureWebPage()
{
    if(mWebView) {
        WebPage *page = qobject_cast<WebPage*>(mWebView->page());
        page->setAllowDialogs(mItem.allowDialogs());
        page->setUserAgentType(mItem.userAgent());
        page->setHostUserAgent(mItem.hostUserAgent());
        page->setNetworkAccessManager(new NetworkManager(this));
        mWebView->setupNetworkAccessManager();
        mJSO->setHostUserAgent(mItem.hostUserAgent());
        mJSO->setFlashEnabled(mItem.enableFlash());
    }
}

void Browser::processPlayerActiveStatesAfterLoading()
{
    BrowserAction action = BA_UNDEF;
    BrowserCondition condition = 0;

    qDebug() << "Browser::processPlayerActiveStatesAfterLoading: state" << mParentState << "is_ad" << mIsAd << "type" << type();
    if((type() == AceWebBrowser::BTYPE_PREROLL ||
       (type() == BTYPE_OVERLAY && mParentState == AceWebBrowser::BHPS_PLAYING)) &&
       !mItem.startHidden()) {
        action = BA_SHOW;
    }
    else if(type() == AceWebBrowser::BTYPE_PREPLAY ||
            // playing
            (mParentState == AceWebBrowser::BHPS_PLAYING
                && type() == AceWebBrowser::BTYPE_SLIDER) ||
            // paused
            (mParentState == AceWebBrowser::BHPS_PAUSED
                && type() == AceWebBrowser::BTYPE_PAUSE) ||
            // stopped
            (mParentState >= AceWebBrowser::BHPS_STOPPED
                && type() == AceWebBrowser::BTYPE_STOP)) {
        action = BA_SHOW;
    }
    else if(type() == AceWebBrowser::BTYPE_HIDDEN) {
        if(mItem.closeAfterSeconds() > 0) {
            if(!mCloseAfterIntHiddenTimer) {
                mCloseAfterIntHiddenTimer = new QTimer(this);
                mCloseAfterIntHiddenTimer->setSingleShot(true);
                connect(mCloseAfterIntHiddenTimer, SIGNAL(timeout()), SLOT(closeActionTriggered()));
            }
            mCloseAfterIntHiddenTimer->setInterval(mItem.closeAfterSeconds() * 1000);
            mCloseAfterIntHiddenTimer->start();
        }

        if(mItem.showTime() == -1) {
            action = BA_SHOW;
        }
        else if(mItem.showTime() > 0) {
            action = BA_SHOW;

            if(!mHideIntHiddenTimer) {
                mHideIntHiddenTimer = new QTimer(this);
                mHideIntHiddenTimer->setSingleShot(true);
                connect(mHideIntHiddenTimer, SIGNAL(timeout()), SLOT(hideBrowser()));
            }
            mHideIntHiddenTimer->setInterval(mItem.showTime() * 1000);
            mHideIntHiddenTimer->start();
        }
    }
    doAction(action, condition);
}

void Browser::processPlayerActiveStatesAfterStateChanged()
{
    qDebug() << "Browser::processPlayerActiveStatesAfterStateChanged: state" << mParentState << "is_ad" << mIsAd << "type" << type();

    BrowserAction action = BA_UNDEF;
    BrowserCondition condition = 0;

    if(mParentState == AceWebBrowser::BHPS_NONE) { // media changed idle state
        action = BA_CLOSE;
    }
    else if(mParentState == AceWebBrowser::BHPS_PLAYING) { // playing
        if(type() == AceWebBrowser::BTYPE_OVERLAY || type() == AceWebBrowser::BTYPE_SLIDER)
            action = BA_SHOW;
        else if((type() == AceWebBrowser::BTYPE_PAUSE && isVisible())
                || (type() == AceWebBrowser::BTYPE_STOP && isVisible())) {
            action = BA_CLOSE;
            //mItem.clearEventFlags();
        }
    }

    else if(type() == AceWebBrowser::BTYPE_PREPLAY && mParentState == AceWebBrowser::BHPS_PLAYING) {
        action = BA_CLOSE;
    }
    else if(mParentState >= AceWebBrowser::BHPS_STOPPED) { // stopped
        if(type() == AceWebBrowser::BTYPE_OVERLAY || type() == AceWebBrowser::BTYPE_SLIDER) {
            action = BA_HIDE;
            mItem.clearEventFlags();
        }
    }

    doAction(action, condition);
}

void Browser::doAction(BrowserAction action, Browser::BrowserCondition condition)
{
    if(action == BA_UNDEF) return;

    bool willBeExecute = condition ? condition(this) : true;
    if(!willBeExecute) return;

    switch(action) {
    case BA_SHOW:
        showBrowser();
        break;
    case BA_HIDE:
        hideBrowser();
        break;
    case BA_CLOSE:
        closeBrowser();
        break;
    default:
        break;
    }
}

void Browser::load()
{
    if(mWebView) {
        activateBrowserMode(false);
        updateSizing();
        configureWebPage();
        mWebView->load(QUrl(mItem.urlWithId()->url()));
        mIsContentRequested = true;
    }
}

void Browser::handleJSOSendEvent(QString event_name)
{
    emit registerSendEvent(mItem.type(), event_name, mItem.urlWithId()->id());
}

/******************************
 *     Public slots
 ******************************/
void Browser::showBrowser()
{
    if(!mShowAvailable) {
        qDebug() << "Browser::showBrowser: showing disabled" << type();
        return;
    }

    if(!mItem.preload() && mState != BS_LOADED) {
        load();
        return;
    }

    if(mState == BS_LOADED && 
            (!mIsAd || type() == AceWebBrowser::BTYPE_PREROLL || type() == BTYPE_PREPLAY)) {
        qDebug() << "Browser::showBrowser: showing parent visible" << parentWidget()->isVisible();
        emit notifyParentCommandToShow(type());
        mNeedsReshowing = true;
        show();
        raise();
    }
}

void Browser::hideBrowser()
{
    enableNavigationButton(mBackAction, false);
    enableNavigationButton(mFwdAction, false);
    if(mNavigationBar) mNavigationBar->hide();

    hide();
}

void Browser::closeBrowser(bool failed)
{

    mDieing = true;
    if(!mItem.hideRegistered()) {
        emit registerBrowserClosedEvent(mItem.type(), mItem.urlWithId()->id(), failed, mBrowserModeEnabled, mItem.groupId());
        mItem.setHideRegistered(true);
    }
    hideBrowser();
    deleteWebView();
    emit notifyBrowserClosed();

}

void Browser::deferredCloseBrowser(unsigned int seconds)
{
    if(!mDeferredTimer) {
        mDeferredTimer = new QTimer(this);
        mDeferredTimer->setSingleShot(true);
        connect(mDeferredTimer, SIGNAL(timeout()), SLOT(closeActionTriggered()));
    }
    mDeferredTimer->setInterval(seconds * 1000);
    mDeferredTimer->start();
    hideBrowser();
}

/******************************
 *     Player handlers
 ******************************/
void Browser::handleParentSize(const QSize &size)
{
    updateSizing(size);
}

void Browser::handleParentFullscreen(bool isFullscreen)
{
    if(isFullscreen != mParentFullscreen)
    {
        mParentFullscreen = isFullscreen;

        if(mWebView) {
            mWebView->setLinkDelegationPolicy((mParentFullscreen || mBrowserModeEnabled)
                                              ? QWebPage::DontDelegateLinks
                                              : QWebPage::DelegateAllLinks);
        }

        if(isVisible())
            emit notifyBrowserPosition(type(), pos(), size());
        mJSO->setIsInFullscreen(mParentFullscreen);
    }
}

void Browser::handleParentState(int state, bool isAd)
{
    if(state != mParentState)
    {
        mParentState = state;
        mIsAd = isAd;
        processPlayerActiveStatesAfterStateChanged();
    }
}

void Browser::handleParentPauseClicked()
{
    qDebug() << "Browser::handleParentPauseClicked";

    BrowserAction action = BA_UNDEF;
    BrowserCondition condition = 0;

    if(type() == AceWebBrowser::BTYPE_OVERLAY || type() == AceWebBrowser::BTYPE_SLIDER) {
        action = BA_HIDE;
        condition = (BrowserCondition)PauseBrowserAvailable;
    }
    else if(type() == AceWebBrowser::BTYPE_PAUSE) {
        action = BA_SHOW;
        //mShowOnPlayerStateChangedOnly = false;
    }
    doAction(action, condition);
}

void Browser::handleParentStopClicked()
{
    qDebug() << "Browser::handleParentStopClicked";

    BrowserAction action = BA_UNDEF;
    BrowserCondition condition = 0;

    if(type() == AceWebBrowser::BTYPE_STOP) {
        action = BA_SHOW;
    }

    doAction(action, condition);
}

/******************************
 *     Widget Events
 ******************************/
void Browser::showEvent(QShowEvent *event)
{
    mNeedsReshowing = false;
    if(mVisiabilityProcessingEnable) {
        emit notifyBrowserVisiabilityChanged(mItem.type(), true);
        
        // reset hideRegistered when browser is shown
        mItem.setHideRegistered(false);

        mJSO->setVisiability(true);
        if(mWebView) {
            WebPage *page = qobject_cast<WebPage*>(mWebView->page());
            page->setDialogsCanBeShown(true);
        }
        if(!mItem.shownRegistered()) {
            emit registerBrowserShownEvent(mItem.type(), mItem.urlWithId()->id());
            mItem.setShownRegistered(true);
        }

        emit notifyBrowserPosition(type(), pos(), size());
    }
    event->accept();
}

void Browser::hideEvent(QHideEvent *event)
{
    mNeedsReshowing = false;
    if(mVisiabilityProcessingEnable) {
        emit notifyBrowserVisiabilityChanged(mItem.type(), false);

        mJSO->setVisiability(false);
        if(mWebView) {
            WebPage *page = qobject_cast<WebPage*>(mWebView->page());
            page->setDialogsCanBeShown(false);
        }
        if(type() != BTYPE_HIDDEN) {
            if(!mItem.hideRegistered()) {
                emit registerBrowserHideEvent(mItem.type(), mItem.urlWithId()->id());
                mItem.setHideRegistered(true);
            }
        }
    }
    event->accept();
}

void Browser::resizeEvent(QResizeEvent *event)
{
    emit notifyBrowserSizeChanged(event->size());
    event->accept();
}

/******************************
 *     Positioning
 ******************************/
void Browser::updateSizing()
{
    updateSizing(QSize(parentWidget()->width(), parentWidget()->height()));
}

void Browser::updateSizing(const QSize &size)
{
    if(type() == BTYPE_WEBSTAT) {
        return;
    }
    
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

    int bottomSpace = (type() == BTYPE_PREROLL || type() == BTYPE_PREPLAY)
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

    qDebug() <<"Browser::updateSizing: Parent" << size << "Browser" << minimumSize();
    updatePosition();
    updateScrollbars();
}

void Browser::updatePosition()
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
    if(isVisible())
        emit notifyBrowserPosition(type(), pos(), size());
    qDebug() << "Browser::updatePosition: Moving to" << x << y << "with size" << maximumWidth() << maximumHeight();
}

void Browser::updateScrollbars()
{
    bool scrollbarsEnable = false;
    if(type() != BTYPE_PREROLL && type() != AceWebBrowser::BTYPE_HIDDEN) {
        int pWidth = parentWidget()->width();
        if(width() >= pWidth || (pWidth > width() && mItem.width() == 0)) {
            scrollbarsEnable = true;
        }

        int pHeight = parentWidget()->height();
        if(height() >= pHeight || (pHeight > height() && mItem.height() == 0)) {
            scrollbarsEnable &= true;
        }
        else {
            scrollbarsEnable &= false;
        }
    }

    if(mWebView) {
        mWebView->setScrollbarsEnable(scrollbarsEnable);
    }
}

void Browser::handleJSOFillParentSizeCommand()
{
    mItem.setFixedBottomSpace(0);
    mItem.setFixedFullscreenBottomSpace(0);
    mItem.setSize(0,0);
    updateSizing();
}

void Browser::handleJSOResizeCommand(QSize newSz)
{
    mItem.setSize(newSz.width(), newSz.height());
    updateSizing();
}

/******************************
 *     Navigation Bar
 ******************************/
QString Browser::setupButtonStyle(const QString &name, int width, int height, const QString &horalign, bool hasDisableState)
{
    QString ss = QString(
        "QToolButton {"
        "   border: none;"
        "   background: url(:/images/%1) top %2 no-repeat;"
        "   margin: 0px;"
        "   width: %3px;"
        "   height: %4px;"
        "}"
        "QToolButton:hover {"
        "   background: url(:/images/%1_h) top %2 no-repeat;"
        "}" )
            .arg(name)
            .arg(horalign)
            .arg(QString::number(width))
            .arg(QString::number(height));
    if(hasDisableState) {
        ss.append( QString(
        "QToolButton:disabled {"
        "   background: url(:/images/%1_d) top %2 no-repeat;"
        "}" ).arg(name).arg(horalign));
    }
    return ss;
}

void Browser::createNavigationBar()
{
    if(mNavigationBar) return;

    mNavigationBar = new QToolBar(this);
    mNavigationBar->setStyleSheet("QToolBar { border: none; background-color: #434343; }");
    mNavigationBar->setMinimumHeight(36);
    mNavigationBar->setMaximumHeight(36);

    mBackAction = new QAction(this);
    connect(mBackAction, SIGNAL(triggered()), SLOT(backActionTriggered()));
    mNavigationBar->addAction(mBackAction);
    QToolButton *backButton =(QToolButton *)mNavigationBar->widgetForAction(mBackAction);
    if(backButton) {
        backButton->setStyleSheet(setupButtonStyle("back", 42, 36));
        backButton->setToolTip(tr("Back"));
        backButton->setEnabled(false);
    }

    mFwdAction = new QAction(this);
    connect(mFwdAction, SIGNAL(triggered()), SLOT(fwdActionTriggered()));
    mNavigationBar->addAction(mFwdAction);
    QToolButton *nextButton =(QToolButton *)mNavigationBar->widgetForAction(mFwdAction);
    if(nextButton) {
        nextButton->setStyleSheet(setupButtonStyle("next", 42, 36, "center"));
        nextButton->setToolTip(tr("Next"));
        nextButton->setEnabled(false);
    }

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mNavigationBar->addWidget(spacer);

    mCloseAction = new QAction(this);
    connect(mCloseAction, SIGNAL(triggered()), SLOT(closeActionTriggered()));
    mNavigationBar->addAction(mCloseAction);
    QToolButton *closeButton =(QToolButton *)mNavigationBar->widgetForAction(mCloseAction);
    if(closeButton) {
        closeButton->setStyleSheet(setupButtonStyle("close", 70, 36, "right", false));
        closeButton->setToolTip(tr("Close"));
    }

    mNavigationBar->setVisible(false);
}

void Browser::enableNavigationButton(QAction *action, bool value)
{
    if(!mNavigationBar) return;
        
    QToolButton *button =(QToolButton *)mNavigationBar->widgetForAction(action);
    if(button) {
        button->setEnabled(value);
    }
}

void Browser::backActionTriggered()
{
    mWebView->changeReferer(WebView::RT_CURRENT);
    mWebView->page()->triggerAction(QWebPage::Back);
}

void Browser::fwdActionTriggered()
{
    mWebView->changeReferer(WebView::RT_CURRENT);
    mWebView->page()->triggerAction(QWebPage::Forward);
}

void Browser::closeActionTriggered()
{
    closeBrowser();
}

void Browser::updateActionButtons()
{
    enableNavigationButton(mBackAction, mWebView->history()->canGoBack());
    enableNavigationButton(mFwdAction, mWebView->history()->canGoForward());
}

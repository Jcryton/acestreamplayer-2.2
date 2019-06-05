#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "qt4.hpp"

#include "acewebbrowser/browsermanager.hpp"
#include "acewebbrowser/cookiemanager.hpp"

using namespace AceWebBrowser;

BrowserManager::BrowserManager(QObject *parent) :
    QObject(parent)
{
    CookieManager *cookieManager = CookieManager::getInstanse();
    CookieManager::holdInstanse(cookieManager);
}

BrowserManager::~BrowserManager()
{
    for(int i = mBrowsers.size()-1; i >= 0; --i) {
        deleteBrowser(i);
    }
    mFilterCache.clear();
    CookieManager *cookieManager = CookieManager::getInstanse();
    CookieManager::releaseInstance(cookieManager);
}

Browser *BrowserManager::createBrowser(const LoadItem &item, QWidget *parent)
{
    if(item.type() != AceWebBrowser::BTYPE_WEBSTAT) {
        Browser *browser = getBrowser(item.type(), item.groupId());
        if(browser) {
            return browser;
        }
    }

    Browser *newBrowser = new Browser(item, this, parent);
    connect(newBrowser, SIGNAL(notifyBrowserClosed()), SLOT(handleBrowserClosed()));

    mBrowsers.append(newBrowser);
    qDebug() << "BrowserManager::createBrowser: Adding new browser to manager. Type" << item.type() << "Size" << mBrowsers.size();
    return newBrowser;
}

void BrowserManager::closeBrowser(BrowserType type)
{
    int index = getBrowserIndex(type);
    deleteBrowser(index);
}

Browser *BrowserManager::getBrowser(BrowserType type, int group)
{
    Browser *browser = NULL;
    foreach (Browser *b, mBrowsers) {
        if(b->type() == type && !b->dieing() && b->groupId() == group) {
            browser = b;
            break;
        }
    }
    return browser;
}

void BrowserManager::updateBrowsersOnVoutChanged(bool hasVout)
{
    if(hasVout) {
        qDebug() << "BrowserManager::updateBrowsersOnVoutChanged: updating browser visiability on vout appeares";
        /*Browser *overlay = getBrowser(BTYPE_OVERLAY);
        if(overlay) {
            overlay->showBrowser();
        }
        Browser *slider = getBrowser(BTYPE_SLIDER);
        if(slider) {
            slider->showBrowser();
        }*/
        foreach (Browser *b, mBrowsers) {
            if(b->needsReshowing()) {
                b->showBrowser();
            }
        }
    }
}

Browser *BrowserManager::getVisibleBrowser()
{
    Browser *browser = NULL;
    foreach (Browser *b, mBrowsers) {
        if(b->isVisible()) {
            browser = b;
            break;
        }
    }
    return browser;
}

int BrowserManager::getBrowserIndex(BrowserType type)
{
    int index = -1;
    for(int i = 0; i < mBrowsers.size(); ++i) {
        if(mBrowsers.at(i)->type() == type) {
            index = i;
            break;
        }
    }
    return index;
}

void BrowserManager::deleteBrowser(int index)
{
    if(index >= 0 && index < mBrowsers.size()) {
        Browser *browser = mBrowsers.at(index);
        mBrowsers.removeAt(index);
        qDebug() << "BrowserManager::deleteBrowser: Removing browser from manager. Type" << browser->type() << "Size" << mBrowsers.size();
        browser->deleteLater();
    }
}

bool BrowserManager::isVisible() const
{
    bool vis = false;
    foreach (Browser *b, mBrowsers) {
        if(b->isVisible() && b->type() != BTYPE_WEBSTAT) {
            vis = true;
            break;
        }
    }
    return vis;
}

bool BrowserManager::hasBrowser(BrowserType type)
{
    Browser *browser = getBrowser(type);
    return browser && !browser->dieing();
}

void BrowserManager::handleBrowserClosed()
{
    if(sender()) {
        Browser *browser = qobject_cast<Browser*>(sender());
        if(browser) {
            if(mBrowsers.removeOne(browser)) {
                qDebug() << QString("BrowserManager:handleBrowserClosed: removing browser from manager: type=%1 url=%2 count=%3")
                           .arg(QString::number(browser->type()))
                           .arg(browser->baseUrl())
                           .arg(QString::number(mBrowsers.size()));
            }
            else {
                qDebug() << "BrowserManager:handleBrowserClosed: no sender browser in manager";
            }
            browser->deleteLater();
        }
    }
}

bool BrowserManager::filterCacheHasValue(QString key)
{
    return mFilterCache.contains(key);
}

bool BrowserManager::filterCacheGetValue(QString key)
{
    return mFilterCache.value(key);
}

void BrowserManager::filterCacheAddValue(QString key, bool value)
{
    mFilterCache.insert(key, value);
}
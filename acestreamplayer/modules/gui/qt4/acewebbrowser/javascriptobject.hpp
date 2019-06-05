#ifndef AWB_JAVASCRIPTOBJECT_H
#define AWB_JAVASCRIPTOBJECT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QObject>
#include <QSize>

namespace AceWebBrowser {

class JavaScriptObject : public QObject
{
    Q_OBJECT
public:
    explicit JavaScriptObject(QObject *parent = 0);

    void setIsInFullscreen(bool value);
    void setFlashEnabled(bool value);
    void setVisiability(bool value);
    void setHostUserAgent(const QString &value);

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
    void browserClose();
    void browserHide();
    void browserShow();
    void browserCloseAfter(unsigned int);

    void linkOpen(QString url, bool openInNewWindow, bool openInAceWeb = false, QString arguments = "");
    void playerPlay();
    void playerPause();
    void playerStop();
    void linkOpenLater(QString url, bool high_priority, bool openInAceWeb = false, QString arguments = "");
    void fillPlayerSize();
    void playerSetFullscreen(bool);
    void browserSetSize(unsigned int, unsigned int);
    void sendEvent(QString event_name);
    void playerToggleFullscreen();

    bool playerIsFullscreen();
    bool browserIsVisible();
    QString pluginVersion();
    bool flashEnabled();
    QString getHostUserAgent();
    unsigned int playerWidth() { return mPlayerSize.width(); }
    unsigned int playerHeight() { return mPlayerSize.height(); }
    unsigned int browserWidth() { return mBrowserSize.width(); }
    unsigned int browserHeight() { return mBrowserSize.height(); }

    void debug(QString msg);

    void handlePlayerSizeChanged(const QSize &);
    void handleBrowserSizeChanged(const QSize &);
};

}

#endif // AWB_JAVASCRIPTOBJECT_H

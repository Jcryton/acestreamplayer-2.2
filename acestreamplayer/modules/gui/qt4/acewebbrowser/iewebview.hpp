#ifndef IEWEBVIEW_H
#define IEWEBVIEW_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QAxWidget>
#include <QUrl>

#include "windows.h"
#include "exdisp.h"
#include "mshtml.h"

namespace AceWebBrowser {

class IEWebView : public QAxWidget
{
public:
    IEWebView(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~IEWebView();

    void connectEmbedElements();
    void connectEmbedElements(IDispatch *);
    //void AddCustomObject(IDispatch *, IDispatch *, QString);

    void load(const QUrl& url);

protected:
    virtual bool translateKeyEvent(int message, int keycode) const;

private:
    IWebBrowser2 *getWebBrowser();

    QUrl mLastChangedUrl;
    IWebBrowser2 *mWebBrowser;
};

}

#endif // IEWEBVIEW_H

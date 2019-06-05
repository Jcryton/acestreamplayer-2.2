#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "acewebbrowser/iewebview.hpp"
#include "acewebbrowser/iebrowser2.hpp"
#include "acewebbrowser/exceptions.hpp"
#include <QUuid>
#include <QAxObject>

using namespace AceWebBrowser;

#define CLSID_InternetExplorer "{8856F961-340A-11D0-A96B-00C04FD705A2}"

IEWebView::IEWebView(QWidget *parent, Qt::WindowFlags flags)
    : QAxWidget(parent, flags)
    , mWebBrowser(0)
{
    if(!setControl(CLSID_InternetExplorer)) {
        throw new Exception("Cannot initialize COM object");
    }

    dynamicCall("SetSilent(bool)", true);
    //mWebBrowser = getWebBrowser();
    //if(mWebBrowser) {
    //    mWebBrowser->put_Silent(VARIANT_TRUE);
    //}
}

IEWebView::~IEWebView()
{
    if(mWebBrowser) {
        mWebBrowser->Release();
    }
    qDebug() << "IEWebView::~IEWebView";
}

/*void IEWebView::AddCustomObject(IDispatch *frame, IDispatch *customObject, QString customName)
{
    qDebug() << "IEWebView::AddCustomObject";

    if(!frame) {
        qDebug() << "IEWebView::AddCustomObject: missing frame";
        return;
    }

    IDispatchEx *winEx;
    QAxObject obj((IUnknown*)frame);
    QAxObject *doc = obj.querySubObject("Document");
    if(!doc) {
        qDebug() << "IEWebView::AddCustomObject: missing doc";
        return;
    }
    QAxObject* win = doc->querySubObject("parentWindow");
    if(!win) {
        qDebug() << "IEWebView::AddCustomObject: missing win";
        return;
    }
    win->queryInterface(QUuid(IID_IDispatchEx), (void**)&winEx);
    if(!winEx) {
        qDebug() << "IEWebView::AddCustomObject: missing winEx";
        return;
    }

    int lenW = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, customName.toStdString().c_str(), -1, NULL, 0);
    BSTR objName = SysAllocStringLen(0, lenW);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, customName.toStdString().c_str(), -1, objName, lenW);

    DISPID dispid;
    HRESULT hr = winEx->GetDispID(objName, fdexNameEnsure, &dispid);

    SysFreeString(objName);

    if (FAILED(hr)) {
        return;
    }

    DISPID namedArgs[] = {DISPID_PROPERTYPUT};
    DISPPARAMS params;
    params.rgvarg = new VARIANT[1];
    params.rgvarg[0].pdispVal = customObject;
    params.rgvarg[0].vt = VT_DISPATCH;
    params.rgdispidNamedArgs = namedArgs;
    params.cArgs = 1;
    params.cNamedArgs = 1;

    hr = winEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);
    winEx->Release();

    if (FAILED(hr)) {
        qDebug() << "IEWebView::AddCustomObject: failed";
        return;
    }

    qDebug() << "IEWebView::AddCustomObject: success";
}*/

void IEWebView::connectEmbedElements()
{
    IDispatch *dispatch = 0;
    queryInterface(QUuid(IID_IDispatch), (void**)&dispatch);
    connectEmbedElements(dispatch);
}

void IEWebView::connectEmbedElements(IDispatch *dispatch)
{
    if(!dispatch) {
        return;
    }

    IEBrowser2 *parentBrowser = qobject_cast<IEBrowser2*>(parentWidget());
    if(!parentBrowser) {
        qDebug() << "IEWebView::connectEmbedElements: no parent browser";
        return;
    }

    QStringList embedScripts = parentBrowser->embedScripts();
    QString embedCode = parentBrowser->embedCode();

    if(embedCode.compare("") && embedScripts.length() == 0) {
        qDebug() << "IEWebView::connectEmbedElements: nothing to embed";
    }

    QAxObject obj((IUnknown*)dispatch);
    QAxObject *document = obj.querySubObject("Document");
    if(!document) {
        qDebug() << "IEWebView::connectEmbedElements: missing doc";
        return;
    }
    QAxObject *window = document->querySubObject("parentWindow");
    if(!window) {
        qDebug() << "IEWebView::connectEmbedElements: missing win";
        return;
    }
    IHTMLWindow2 *htmlwindow = 0;
    window->queryInterface(QUuid(IID_IHTMLWindow2), (void**)&htmlwindow);
    if(!htmlwindow) {
        qDebug() << "IEWebView::connectEmbedElements: missing html window";
        return;
    }

    foreach(QString script, embedScripts) {
        QString js = QString(ADD_JS_SCRIPT_JS).replace("%SCRIPT_URL%", script);
        BSTR _script = SysAllocString(js.toStdWString().c_str());
        VARIANT v; VariantInit(&v);
        htmlwindow->execScript(_script, 0, &v);
        VariantClear(&v);
        SysFreeString(_script);
    }
    if(embedCode.compare("")) {
        QString js = QString(ADD_JS_CODE_JS).replace("%SCRIPT_CODE%", embedCode);
        BSTR _script = SysAllocString(js.toStdWString().c_str());
        VARIANT v; VariantInit(&v);
        htmlwindow->execScript(_script, 0, &v);
        VariantClear(&v);
        SysFreeString(_script);
    }

    qDebug() << "IEWebView::connectEmbedElements: success";
}

void IEWebView::load(const QUrl &url)
{
    qDebug() << "IEWebView::load: url=" << url.toString();
    mLastChangedUrl = url;
    dynamicCall("Navigate(QString)", url.toString());
    /*if(mWebBrowser) {
        BSTR bstrURL = SysAllocString(url.toString().toStdWString().c_str());
        mWebBrowser->Navigate(bstrURL, 0, 0, 0, 0);
        SysFreeString(bstrURL);
    }*/
}

IWebBrowser2 *IEWebView::getWebBrowser()
{
    IWebBrowser2 *wb = 0;
    queryInterface(QUuid(IID_IWebBrowser2), (void**)&wb);
    return wb;
}

bool IEWebView::translateKeyEvent(int message, int keycode) const
{
    if (message >= WM_KEYFIRST && message <= WM_KEYLAST)
        return true;
    else
        return QAxWidget::translateKeyEvent(message, keycode);
}





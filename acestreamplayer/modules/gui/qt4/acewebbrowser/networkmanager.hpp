#ifndef AWB_NETWORKMANAGER_H
#define AWB_NETWORKMANAGER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QNetworkAccessManager>
#include "acewebbrowser/defines.hpp"

namespace AceWebBrowser {

class Browser;
class NetworkManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit NetworkManager(Browser *browser, QObject *parent = 0);

    void setReferer(const QString&);
protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);

private:
    QList<QString> mTrustedSSLHosts;
    Browser *mBrowser;
    QString mReferer;
    QNetworkAccessManager *mFilterNAM;

private slots:
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &error);
#endif
};

}

#endif // AWB_NETWORKMANAGER_H

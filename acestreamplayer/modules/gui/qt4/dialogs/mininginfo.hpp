#ifndef QVLC_MININGINFO_DIALOG_H_
#define QVLC_MININGINFO_DIALOG_H_ 1

#include "qt4.hpp"
#include <QWidget>

class MiningInfoWidget : public QWidget
{
    Q_OBJECT
public:
    static void ShowMiningDialog( QWidget *_p, intf_thread_t *p_intf, const QString &type, const QString &text, 
            int buttons,
            int action1, const QString &text1, const QString &url1, 
            int action2, const QString &text2, const QString &url2 )
    {
        new MiningInfoWidget( _p, p_intf, type, text, buttons, action1, text1, url1, action2, text2, url2 );
    }
    MiningInfoWidget( QWidget *, intf_thread_t *, const QString&, const QString&, int, int, const QString&, const QString&, int, const QString&, const QString& );
    
private:
    void processButtonClick(int);

    intf_thread_t *p_intf;
    int mAction1;
    int mAction2;
    QString mBtn1Url;
    QString mBtn2Url;
    QString mType;
    
private slots:
    void button0Clicked();
    void button1Clicked();
};

#endif
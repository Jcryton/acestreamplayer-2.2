#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "dialogs/mininginfo.hpp"
#include "actions_manager.hpp" 
#include "input_manager.hpp" 

#include <QString>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

MiningInfoWidget::MiningInfoWidget(QWidget *_p, intf_thread_t *_p_intf, const QString &_type, const QString &_text, 
    int _buttons,
    int _action1, const QString &_text1, const QString &_url1, 
    int _action2, const QString &_text2, const QString &_url2 ) : 
    QWidget( _p )
  , p_intf(_p_intf)
  , mType(_type)
  , mAction1(_action1)
  , mAction2(_action2)
  , mBtn1Url(_url1)
  , mBtn2Url(_url2)
{
    setWindowTitle("");
    setWindowModality( Qt::ApplicationModal );
    setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint );
    setAttribute( Qt::WA_DeleteOnClose );
    
    QVBoxLayout *main = new QVBoxLayout(this);
    QLabel *lblText = new QLabel(this);
    lblText->setMaximumWidth(450);
    lblText->setWordWrap(true);
    lblText->setOpenExternalLinks(true);
    lblText->setText(_text);
    main->addWidget(lblText);
    
    QHBoxLayout *loutbuttons = new QHBoxLayout(this);
    
    if(_buttons == 0) {
        QPushButton *button = new QPushButton(this);
        button->setText("Ok");
        CONNECT(button, clicked(), this, close());
        loutbuttons->addWidget(button);
    }
    else if(_buttons == 1) {
        QPushButton *button1 = new QPushButton(this);
        button1->setText(_text1);
        CONNECT(button1, clicked(), this, button0Clicked());
        loutbuttons->addWidget(button1);
    }
    else if(_buttons == 2) {
        QPushButton *button1 = new QPushButton(this);
        QPushButton *button2 = new QPushButton(this);
        button1->setText(_text1);
        button2->setText(_text2);
        CONNECT(button1, clicked(), this, button0Clicked());
        CONNECT(button2, clicked(), this, button1Clicked());
        loutbuttons->addWidget(button1);
        loutbuttons->addWidget(button2);
    }
    
    main->addLayout(loutbuttons);
    setLayout(main);
    
    if(_buttons >= 0 && _buttons <= 2) {
        setVisible(true);
        
        if( _p ) {
            move(_p->x() + _p->width() / 2 - width() / 2, _p->y() + _p->height() / 2 - height() / 2);
        }
    }
}

void MiningInfoWidget::processButtonClick(int btn)
{
    int action = btn == 1 ? mAction1 : mAction2;
    QString url = btn == 1 ? mBtn1Url : mBtn2Url;
    if(action & P2P_IW_BTN_ACTION_MINIGACTIVATE) {
        p2p_UserDataMining(THEP2P, 1);
    }
    if(action & P2P_IW_BTN_ACTION_SENDEVENT) {
        p2p_InfoWindowsResponse(THEP2P, qtu(mType), btn);
    }
    if(action & P2P_IW_BTN_ACTION_STOP) {
        THEMIM->stop();
    }
    if(action & P2P_IW_BTN_ACTION_PLAY) {
        THEAM->play();
    }
    if(action & P2P_IW_BTN_ACTION_OPENLINK) {
        if(!url.isEmpty()) {
            QDesktopServices::openUrl(QUrl(url));
        }
    }
    if(action & P2P_IW_BTN_ACTION_CLOSE) {
        close();
    }
}

void MiningInfoWidget::button0Clicked()
{
    processButtonClick(1);
}

void MiningInfoWidget::button1Clicked()
{
    processButtonClick(2);
}
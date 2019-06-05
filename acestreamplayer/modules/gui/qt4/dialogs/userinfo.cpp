#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "dialogs/userinfo.hpp"

#include <QString>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QKeyEvent>

UserInfoWidget::UserInfoWidget(QWidget *_p, intf_thread_t *_p_intf ) : QWidget( _p ), p_intf( _p_intf )
{
    setWindowTitle("");
    setWindowModality( Qt::ApplicationModal );
    setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint );
    setAttribute( Qt::WA_DeleteOnClose );
    
    QGridLayout *main = new QGridLayout( this );
    
    QLabel *lblDescr = new QLabel(qtr("To start watching specify your gender and age"));
    main->addWidget(lblDescr, 0,0,1,3);
    
    QLabel *lblSpace = new QLabel("");
    main->addWidget(lblSpace, 1,0,1,3);
    
    QLabel *lblGender = new QLabel(QString(qtr("Gender")).append(":"));
    rbMale = new QRadioButton(qtr("Male"));
    rbFemale = new QRadioButton(qtr("Female"));
    main->addWidget(lblGender, 2,0,1,1);
    main->addWidget(rbMale, 2,1,1,1);
    main->addWidget(rbFemale, 2,2,1,1);
    
    QLabel *lblAge = new QLabel(QString(qtr("Age")).append(":"));
    cbAge = new QComboBox;
    cbAge->addItem("", 0);
    cbAge->addItem(QString(qtr("Under")).append(" 13"), 1);
    cbAge->addItem("13-17", 2);
    cbAge->addItem("18-21", 3);
    cbAge->addItem("22-25", 9);
    cbAge->addItem("26-30", 4);
    cbAge->addItem("31-36", 10);
    cbAge->addItem("37-44", 5);
    cbAge->addItem("45-54", 6);
    cbAge->addItem("55-64", 7);
    cbAge->addItem(QString(qtr("Above")).append(" 64"), 8);
    main->addWidget(lblAge, 3,0,1,1);
    main->addWidget(cbAge, 3,1,1,2);
    
    lblStatus = new QLabel("");
    QPushButton *pbOkay = new QPushButton("Ok");
    main->addWidget(lblStatus, 4,0,1,2);
    main->addWidget(pbOkay, 4,2,1,1);
    
    setLayout(main);
    
    CONNECT(pbOkay, clicked(), this, okayClicked());
    
    setVisible(true);

    if( _p ) {
        move(_p->x() + _p->width() / 2 - width() / 2, _p->y() + _p->height() / 2 - height() / 2);
    }
    
    pbOkay->setFocus();
}

void UserInfoWidget::keyPressEvent( QKeyEvent *e )
{
    if(e->key() == Qt::Key_Enter) {
        okayClicked();
        e->accept();
    }
    else
        e->ignore();
}

void UserInfoWidget::okayClicked()
{
    int gender = rbMale->isChecked() ? 1 : rbFemale->isChecked() ? 2 : 0;
    if (!gender) {
        lblStatus->setText("<span style='color:#FF0000'>"+QString(qtr("Specify your gender"))+"</span>");
        return;
    }
    int age = cbAge->currentIndex();
    if (!age) {
        lblStatus->setText("<span style='color:#FF0000'>"+QString(qtr("Specify your age"))+"</span>");
        return;
    }
    p2p_UserData(THEP2P, gender, age);
    close();
}
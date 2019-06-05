#ifndef QVLC_USERINFO_DIALOG_H_
#define QVLC_USERINFO_DIALOG_H_ 1

#include "qt4.hpp"
#include <QWidget>

class QRadioButton;
class QComboBox;
class QLabel;
class QKeyEvent;
class UserInfoWidget : public QWidget
{
    Q_OBJECT
public:
    static void ShowUserInfoDialog( QWidget *_p, intf_thread_t *p_intf )
    {
        new UserInfoWidget( _p, p_intf );
    }
    UserInfoWidget( QWidget *, intf_thread_t * );

protected:
    virtual void keyPressEvent( QKeyEvent *);
    
private:
    intf_thread_t *p_intf;
    QRadioButton *rbMale;
    QRadioButton *rbFemale;
    QLabel *lblStatus;
    QComboBox *cbAge;
    
private slots:
    void okayClicked();
};

#endif
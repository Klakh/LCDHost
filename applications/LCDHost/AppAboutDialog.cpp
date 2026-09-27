#include "AppAboutDialog.h"

#include "LCDHost.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPixmap>
#include <QString>
#include <QLabel>

AppAboutDialog::AppAboutDialog( QWidget *parent, Qt::WindowFlags f ) : QDialog( parent, f )
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok,Qt::Horizontal,this);

    connect( buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    QHBoxLayout *hbox = new QHBoxLayout();

    QLabel *icon = new QLabel(this);
    icon->setAlignment( Qt::AlignTop | Qt::AlignLeft );
    icon->setPixmap( QPixmap(":/lcdhost/images/lcdhost128.png") );

    QLabel *label = new QLabel(this);
    label->setTextFormat( Qt::RichText );
    label->setOpenExternalLinks(true);

    label->setText(
            tr("LCDHost version %1 ("
#if defined(Q_OS_WIN)
                "win"
#elif defined(Q_OS_MAC)
                "mac"
#elif defined(Q_OS_LINUX)
                "lin"
#else
                "xxx"
#endif
               "%2) changeset %3:%4<br>"
               "Copyright &copy;2009-2016 Johan Lindh (Link Data Stockholm) and contributors<br>"
               "Source code, help and bug reports: <a href=\"https://github.com/LokLakh-s/LCDHost\">github.com/LokLakh-s/LCDHost</a><br>"
               )
            .arg(VERSION)
            .arg(QSysInfo::WordSize)
            .arg(REVISION)
            .arg(CHANGESET)
            );

    hbox->addWidget( icon );
    hbox->addWidget( label );

    layout->addLayout( hbox );
    layout->addWidget( buttonBox );

    show();

    return;
}

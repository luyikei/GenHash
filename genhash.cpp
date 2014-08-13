#include "genhash.h"
#include <kparts/part.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kshell.h>
#include <kapplication.h>
#include <kpluginfactory.h>
#include <kauthorized.h>
#include <kio/netaccess.h>
#include <kparts/fileinfoextension.h>
#include <kfiledialog.h>
#include <QCryptographicHash>
#include <QTextStream>


GenHash::GenHash( QObject* parent, const QVariantList & )
    : KParts::Plugin( parent )
{

    KAction *action = actionCollection()->addAction("generatehash");
    action->setText(i18n( "&Generate MD5 Hash..." ));
    connect(action, SIGNAL(triggered(bool)), SLOT(calcGenHash()));
}

void GenHash::calcGenHash()
{
    KParts::ReadOnlyPart * part = qobject_cast<KParts::ReadOnlyPart *>(parent());

    QString basePath = KFileDialog::getOpenFileName(KUrl("kfiledialog:///konqueror"), i18n("*"), part->widget(), i18n("Open File To make MD5."));

    if (basePath.size()) {
        QFile file(basePath);

        if (!file.open(QIODevice::ReadOnly))
        {
            return;
        }

        QByteArray baseByteArray = file.readAll();

        KMessageBox::information(part->widget(),i18n("Md5 : %1").arg(
                                     QString(QCryptographicHash::hash(baseByteArray, QCryptographicHash::Md5).toHex())));
    }

}

K_PLUGIN_FACTORY(KonqGenHashFactory, registerPlugin<GenHash>();)
K_EXPORT_PLUGIN(KonqGenHashFactory("genhash"))

#include "genhash.moc"


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
#include <QCryptographicHash>

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

    bool ok;
    QString baseString = KInputDialog::getText(i18nc("@title:window", "Generate MD5 Hash"),
                                        i18n("Please enter a text to generate MD5 hash."),
                                        "", &ok, part->widget());
    if (ok) {
        KMessageBox::information(part->widget(),i18n("Md5 : %1").arg(
                                     QString(QCryptographicHash::hash(baseString.toAscii(), QCryptographicHash::Md5).toHex())));
    }

}

K_PLUGIN_FACTORY(KonqGenHashFactory, registerPlugin<GenHash>();)
K_EXPORT_PLUGIN(KonqGenHashFactory("genhash"))

#include "genhash.moc"


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
#include "kdebug.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>

#include <limits.h>

GenHash::GenHash( QObject* parent, const QVariantList & )
    : KParts::Plugin( parent )
{

    KAction *action = actionCollection()->addAction("generatehash");
    action->setText(i18n( "&Generate MD5 Hash..." ));
    connect(action, SIGNAL(triggered(bool)), SLOT(calcGenHash()));

}

void GenHash::sandbox_init()
{
    if (pipe(pipe_fd) < 0) {
        perror("pipe");
        exit(1);
    }
    if (pipe(pipe_result_fd) < 0) {
        perror("pipe");
        exit(1);
    }

    if ((child_pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

}

void GenHash::calcGenHash()
{
    sandbox_init();

    if (child_pid){

        KParts::ReadOnlyPart * part = qobject_cast<KParts::ReadOnlyPart *>(parent());

        QFile file(KFileDialog::getOpenFileName(KUrl("kfiledialog:///konqueror"), i18n("*"), part->widget(), i18n("Open File To make MD5.")));

        if (!file.open(QIODevice::ReadOnly))
        {
            return;
        }

        QByteArray fileByteArray = file.readAll();

        if (fileByteArray.size()) {
            char *s = fileByteArray.data();

            close(pipe_fd[0]);
            for (int i=0;i<fileByteArray.size();i++) {
                if (write(pipe_fd[1], s+i, 1) < 0) {
                    perror("write");
                    exit(1);
                }
                kDebug() << "p:" << i;
            }
            close(pipe_fd[1]);

            int count;
            char c;
            QByteArray resultByteArray="";

            kDebug() << "w finish";
            close(pipe_result_fd[1]);

            int i=0;
            while ((count = read(pipe_result_fd[0], &c, 1)) > 0) {
                resultByteArray.append(c);
                i++;
            }
            close(pipe_result_fd[0]);

            //KMessageBox::information(part->widget(),i18n("Md5 : %1").arg(QString(resultByteArray)));

            kDebug() << resultByteArray.size();
            //kDebug() << resultByteArray;
            if(waitpid(child_pid, &status, 0) < 0){
                perror("wait");
                exit(1);
            }
            kDebug() << "wait a";
        }
    }else{
        calcMD5();
    }
}

void GenHash::calcMD5()
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    char c;
    int count,i=0;

    close(pipe_fd[1]);
    while ((count = read(pipe_fd[0], &c, 1)) > 0) {
        kDebug() << count;
        kDebug() << i;
        hash.addData(&c, 1);
        i++;
    }
    close(pipe_fd[0]);

    close(pipe_result_fd[0]);
    char *s = hash.result().toHex().data();
    for (int i=0;i<128;i++) {
        if (write(pipe_result_fd[1], s+i, 1) < 0) {
            perror("write");
            exit(1);
        }
        kDebug() << "cr:" << i;
    }
    close(pipe_result_fd[1]);

    exit(0);
}

K_PLUGIN_FACTORY(KonqGenHashFactory, registerPlugin<GenHash>();)
K_EXPORT_PLUGIN(KonqGenHashFactory("genhash"))

#include "genhash.moc"


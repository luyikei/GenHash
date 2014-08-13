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

        // TODO QFile
        //char s[1024];
        // write file content
        {
            close(pipe_fd[0]);
            QByteArray s = file.readAll();
            char *ch = s.data();
            if (write(pipe_fd[1], ch, s.size()) < 0) {
                perror("write");
            }
            close(pipe_fd[1]);
        }
        //read result
        {
            close(pipe_result_fd[1]);
            char result[128];
            if (read(pipe_result_fd[0], &result[0], 128) < 0){
                perror("read");
            }
            close(pipe_result_fd[0]);

            KMessageBox::information(part->widget(),i18n("Md5 : %1").arg(QString(QByteArray(result, 128))));
        }
    }else{
        calcMD5();
    }
}

void GenHash::calcMD5()
{
    QCryptographicHash hash(QCryptographicHash::Md5);

    // read result;
    {
        char s[1024];
        int count;
        close(pipe_fd[1]);
        while ((count = read(pipe_fd[0], &s[0], 1024)) > 0) {
            hash.addData(s, count);
        }
        close(pipe_fd[0]);
    }

    char* result = hash.result().toHex().data();

    //write result
    {

        close(pipe_result_fd[0]);

        write(pipe_result_fd[1], result, 128);

        close(pipe_result_fd[1]);

    }

    exit(0);
}

K_PLUGIN_FACTORY(KonqGenHashFactory, registerPlugin<GenHash>();)
K_EXPORT_PLUGIN(KonqGenHashFactory("genhash"))

#include "genhash.moc"


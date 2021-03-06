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

#include <cap-ng.h>

#include "seccomp.h"
#include <sys/prctl.h>

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

    // cannot excute execve
    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
    kDebug() << "execve" << execve("ls",NULL,NULL);

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

        scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 3,
                                   SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)pipe_fd[0]),
                                   SCMP_A1(SCMP_CMP_EQ, (scmp_datum_t)buff),
                                   SCMP_A2(SCMP_CMP_LE, 1024));
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
                                    SCMP_CMP(0, SCMP_CMP_EQ, (scmp_datum_t)pipe_result_fd[1]));

        seccomp_load(ctx);
        seccomp_release(ctx);

        calcMD5();
    }
}

void GenHash::calcMD5()
{
    QCryptographicHash hash(QCryptographicHash::Md5);

    // read result;
    {
        int count;
        close(pipe_fd[1]);
        while ((count = read(pipe_fd[0], &buff[0], 1024)) > 0) {
            hash.addData(buff, count);
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


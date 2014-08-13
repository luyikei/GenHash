#ifndef genhash_h
#define genhash_h

#include <kparts/plugin.h>
#include <QString>
#include <QByteArray>

class GenHash : public KParts::Plugin
{
    Q_OBJECT
public:
    GenHash( QObject* parent, const QVariantList & );
    ~GenHash() {}

public slots:
    void calcGenHash();

private:

    void sandbox_init();
    void readByteArryFromFile();

    void calcMD5();

    int pipe_fd[2];
    int pipe_result_fd[2];
    int child_pid;
    int status;

};

#endif

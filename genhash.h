#ifndef genhash_h
#define genhash_h

#include <kparts/plugin.h>

class GenHash : public KParts::Plugin
{
    Q_OBJECT
public:
    GenHash( QObject* parent, const QVariantList & );
    ~GenHash() {}

public slots:
    void calcGenHash();
};

#endif

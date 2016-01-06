#ifndef KISIMPORTFILTER_H
#define KISIMPORTFILTER_H

#include <QObject>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

class KisImportFilter : public QObject, public KisShared
{
    Q_OBJECT
public:
    explicit KisImportFilter(QObject *parent = 0);


};

typedef KisSharedPtr<KisImportFilter> KisImportFilterSP;


#endif // KISIMPORTFILTER_H

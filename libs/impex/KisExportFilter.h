#ifndef KISEXPORTFILTER_H
#define KISEXPORTFILTER_H

#include <QObject>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

class KisExportFilter : public QObject, public KisShared
{
    Q_OBJECT
public:
    explicit KisExportFilter(QObject *parent = 0);


};

typedef KisSharedPtr<KisExportFilter> KisExportFilterSP;

#endif // KISEXPORTFILTER_H

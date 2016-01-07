/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISEXPORTFILTER_H
#define KISEXPORTFILTER_H

#include <KisImpExBase.h>

class KisDocument;
class KoUpdater;

/**
 * @brief The KisExportFilter class is the base class for all export filters
 */
class KisExportFilter : public KisImpExBase
{
    Q_OBJECT
public:
    explicit KisExportFilter(const KoID &id, const QStringList &mimeTypes, const QStringList &extensions, QObject *parent = 0);

    /**
     * Export the image in the given KisDocument to the specified filename.
     *
     * @param document: the document to be saved
     * @param filename: The filename is a full and absolute path.
     * @param progressUpdater: to pass on the progress the export is making
     * @returns and empty QStringList. If saving failed, returns a list of all the errors encountered
     */
    QStringList save(const KisDocument *document, const QString &fileName, KoUpdater* progressUpdater) const;

protected:

    virtual QStringList saveImpl(const KisDocument *document, const QString &fileName, KoUpdater* progressUpdater) const = 0;
};

typedef KisSharedPtr<KisExportFilter> KisExportFilterSP;

#endif // KISEXPORTFILTER_H

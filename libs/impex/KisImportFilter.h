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
#ifndef KISIMPORTFILTER_H
#define KISIMPORTFILTER_H

#include <KisImpExBase.h>

class KisDocument;
class KoUpdater;

class KisImportFilter : public KisImpExBase
{
    Q_OBJECT
public:
    explicit KisImportFilter(const KoID &id, const QStringList &mimeTypes, const QStringList &extensions, QObject *parent = 0);

    /**
     * Load the given image into the document's KisImage;
     *
     * @param document: the document
     * @param filename: The filename is a full and absolute path.
     * @param progressUpdater: to pass on the progress the loading is making
     * @returns and empty QStringList. If loading failed, returns a list of all the errors encountered
     */
    QStringList load(KisDocument *document, const QString &fileName, KoUpdater* progressUpdater) const;

protected:

    virtual QStringList loadImpl(KisDocument *document, const QString &fileName, KoUpdater* progressUpdater) const = 0;

};

typedef KisSharedPtr<KisImportFilter> KisImportFilterSP;


#endif // KISIMPORTFILTER_H

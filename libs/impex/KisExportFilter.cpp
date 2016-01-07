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
#include "KisExportFilter.h"

KisExportFilter::KisExportFilter(const KoID &id, const QStringList &mimeTypes, const QStringList &extensions, QObject *parent)
    : KisImpExBase(id, mimeTypes, extensions, parent)
{

}

QStringList KisExportFilter::save(const KisDocument *document, const QString &fileName, KoUpdater *progressUpdater) const
{
    // Create a temporary filename
    // Call saveImpl
    QStringList errors = saveImpl(document, fileName, progressUpdater);
    // Rename as atomically as possible to the real target filename
    // Delete the temporary file

    return errors;
}

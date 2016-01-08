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
#ifndef KISEXPORTFILTERREGISTRY_H
#define KISEXPORTFILTERREGISTRY_H

#include <QScopedPointer>

#include <KoGenericRegistry.h>

#include <KisExportFilter.h>

class KisExportFilterRegistry : public QObject, public KoGenericRegistry<KisExportFilterSP>
{
public:
    KisExportFilterRegistry();
    virtual ~KisExportFilterRegistry();
    static KisExportFilterRegistry *instance();

    QStringList &exportMimeTypes() const;
    KisExportFilterSP exportFilterByMime(const QString &mimeType) const;

    QStringList &exportExtensions() const;
    KisExportFilterSP exportFilterByExtension(const QString &extension) const;

private:
    KisExportFilterRegistry(const KisExportFilterRegistry &);
    KisExportFilterRegistry &operator=(const KisExportFilterRegistry &);

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KISIMPORTFILTERREGISTRY_H

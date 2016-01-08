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
#include "KisExportFilterRegistry.h"
#include <KoPluginLoader.h>

Q_GLOBAL_STATIC(KisExportFilterRegistry, s_instance)

struct KisExportFilterRegistry::Private {
};

KisExportFilterRegistry::KisExportFilterRegistry()
    : KoGenericRegistry<KisExportFilterSP>()
{
    KoPluginLoader::instance()->load("Krita/ExportFilter", "(Type == 'Service') and ([X-Krita-Version] == 28)");

    Q_FOREACH (const QString & id, keys()) {
        KisExportFilterSP exportFilter = get(id);
        qDebug() << exportFilter->name() << exportFilter->mimeTypes() << exportFilter->extensions();
    }
}


KisExportFilterRegistry::~KisExportFilterRegistry()
{

}

KisExportFilterRegistry *KisExportFilterRegistry::instance()
{
    return s_instance;
}

QStringList &KisExportFilterRegistry::exportMimeTypes() const
{

}

KisExportFilterSP KisExportFilterRegistry::exportFilterByMime(const QString &mimeType) const
{

}

QStringList &KisExportFilterRegistry::exportExtensions() const
{

}

KisExportFilterSP KisExportFilterRegistry::exportFilterByExtension(const QString &extension) const
{

}


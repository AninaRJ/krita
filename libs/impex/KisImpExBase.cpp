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
#include "KisImpExBase.h"

struct KisImpExBase::Private {
    KoID id;
    QStringList mimeTypes;
    QStringList extensions;
};

KisImpExBase::KisImpExBase(const KoID &id, const QStringList &mimeTypes, const QStringList &extensions, QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    d->id = id;
    d->mimeTypes = mimeTypes;
    d->extensions = extensions;
}

KisImpExBase::~KisImpExBase()
{
}

QString KisImpExBase::id() const
{
    return d->id.id();
}

QString KisImpExBase::name() const
{
    return d->id.name();
}

QStringList KisImpExBase::mimeTypes() const
{
    return d->mimeTypes;
}

QStringList KisImpExBase::extensions() const
{
    return d->extensions;
}


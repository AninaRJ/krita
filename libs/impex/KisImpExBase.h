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
#ifndef KISIMPEXBASE_H
#define KISIMPEXBASE_H

#include <QObject>
#include <QScopedPointer>
#include <kis_shared.h>
#include <kis_shared_ptr.h>
#include <KoID.h>


/**
 * @brief The KisImpExBase class is the base class for both import and export filters.
 */
class KisImpExBase : public QObject, public KisShared
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QStringList mimeTYpes READ mimeTypes)
    Q_PROPERTY(QStringList extensions READ extensions)

public:
    explicit KisImpExBase(const KoID &id, const QStringList &mimeTypes, const QStringList &extensions, QObject *parent = 0);
    virtual ~KisImpExBase();

    /// @return Unique identification for this export filter
    QString id() const;

    /// @return User-visible identification for this export filter
    QString name() const;

    /// @returns the list of mimetypes this filter can export
    QStringList mimeTypes() const;

    /// @returns the list of extensions this filter can export
    QStringList extensions() const;

private:

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KISIMPEXBASE_H

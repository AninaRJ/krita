/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "CmykU8ColorSpace.h"

#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>

CmykU8ColorSpace::CmykU8ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<CmykU8Traits>(colorSpaceId(), name,  TYPE_CMYKA_8, cmsSigCmykData, p)
{
    addChannel(new KoChannelInfo(i18n("Cyan"), 0 * sizeof(quint8), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Magenta"), 1 * sizeof(quint8), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Yellow"), 2 * sizeof(quint8), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Black"), 3 * sizeof(quint8), 3, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), Qt::black));
    addChannel(new KoChannelInfo(i18n("Alpha"), 4 * sizeof(quint8), 4, KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));

    init();

    addStandardCompositeOps<CmykU8Traits>(this);
}

bool CmykU8ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8)
        return true;
    else
        return false;
}

KoColorSpace* CmykU8ColorSpace::clone() const
{
    return new CmykU8ColorSpace(name(), profile()->clone());
}

void CmykU8ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const CmykU8Traits::Pixel* p = reinterpret_cast<const CmykU8Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("CMYK");
    labElt.setAttribute("c", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA(p->cyan));
    labElt.setAttribute("m", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA(p->magenta));
    labElt.setAttribute("y", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA(p->yellow));
    labElt.setAttribute("k", KoColorSpaceMaths< CmykU8Traits::channels_type, qreal>::scaleToA(p->black));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void CmykU8ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    CmykU8Traits::Pixel* p = reinterpret_cast<CmykU8Traits::Pixel*>(pixel);
    p->cyan = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("c").toDouble());
    p->magenta = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("m").toDouble());
    p->yellow = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("y").toDouble());
    p->black = KoColorSpaceMaths< qreal, CmykU8Traits::channels_type >::scaleToA(elt.attribute("k").toDouble());
    p->alpha = KoColorSpaceMathsTraits<quint8>::max;
}

void CmykU8ColorSpace::toHSY(QVector <double> channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    //we use HSI here because we can't linearise CMYK, and HSY doesn't work right with...
    CMYKToCMY(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    channelValues[0] = 1.0-channelValues[0];
    channelValues[1] = 1.0-channelValues[1];
    channelValues[2] = 1.0-channelValues[2];
    RGBToHSI(channelValues[0],channelValues[1],channelValues[2], hue, sat, luma);
}

QVector <double> CmykU8ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(5);
    channelValues.fill(1.0);
    HSIToRGB(*hue, *sat, *luma, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[0] = qBound(0.0,1.0-channelValues[0],1.0);
    channelValues[1] = qBound(0.0,1.0-channelValues[1],1.0);
    channelValues[2] = qBound(0.0,1.0-channelValues[2],1.0);
    CMYToCMYK(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    return channelValues;
}

void CmykU8ColorSpace::toYUV(QVector <double> channelValues, qreal *y, qreal *u, qreal *v) const
{
    CMYKToCMY(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    channelValues[0] = 1.0-channelValues[0];
    channelValues[1] = 1.0-channelValues[1];
    channelValues[2] = 1.0-channelValues[2];
    RGBToYUV(channelValues[0],channelValues[1],channelValues[2], y, u, v, 0.33, 0.33, 0.33);
}

QVector <double> CmykU8ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(5);
    channelValues.fill(1.0);
    YUVToRGB(*y, *u, *v, &channelValues[0],&channelValues[1],&channelValues[2], 0.33, 0.33, 0.33);
    channelValues[0] = qBound(0.0,1.0-channelValues[0],1.0);
    channelValues[1] = qBound(0.0,1.0-channelValues[1],1.0);
    channelValues[2] = qBound(0.0,1.0-channelValues[2],1.0);
    CMYToCMYK(&channelValues[0],&channelValues[1],&channelValues[2],&channelValues[3]);
    return channelValues;
}

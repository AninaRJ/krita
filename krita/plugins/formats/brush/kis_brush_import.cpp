/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_brush_import.h"

#include <QCheckBox>
#include <QBuffer>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>
#include <QUrl>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <KisFilterChain.h>
#include <KisDocument.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>

#include <kis_gbr_brush.h>
#include <kis_imagepipe_brush.h>

K_PLUGIN_FACTORY_WITH_JSON(KisBrushImportFactory, "krita_brush_import.json", registerPlugin<KisBrushImport>();)

KisBrushImport::KisBrushImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisBrushImport::~KisBrushImport()
{
}


KisImportExportFilter::ConversionStatus KisBrushImport::convert(const QByteArray& from, const QByteArray& to)
{
    qDebug() << "Brush import! From:" << from << ", To:" << to << m_chain->inputFile();

    if (to != "application/x-krita")
        return KisImportExportFilter::BadMimeType;

    QString filename = m_chain->inputFile();

    if (!filename.isEmpty()) {

        if (!QFile(filename).exists()) {
            return KisImportExportFilter::FileNotFound;
        }


        KisBrush *brush = 0;

        if (from == "image/x-gimp-brush") {
            brush = new KisGbrBrush(filename);
        }
        else if (from == "image/x-gimp-brush-animated") {
            brush = new KisImagePipeBrush(filename);
        }

        if (!brush->load()) {
            return KisImportExportFilter::InvalidFormat;
        }

        if (!brush->valid()) {
            return KisImportExportFilter::InvalidFormat;
        }

        KisDocument * doc = m_chain->outputDocument();

        if (!doc) {
            return KisImportExportFilter::NoDocumentCreated;
        }

        doc->prepareForImport();

        const KoColorSpace *colorSpace = 0;
        if (brush->hasColor()) {
            colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        }
        else {
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace(GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "");
        }

        KisImageWSP image = new KisImage(doc->createUndoStore(), brush->width(), brush->height(), colorSpace, brush->name());

        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255, colorSpace);
        layer->paintDevice()->convertFromQImage(brush->brushTipImage(), 0, 0, 0);
        image->addNode(layer.data(), image->rootLayer().data());

        doc->setCurrentImage(image);
        return KisImportExportFilter::OK;
    }

    return KisImportExportFilter::StorageCreationError;

}
#include "kis_brush_import.moc"

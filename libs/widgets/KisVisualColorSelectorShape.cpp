/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualColorSelectorShape.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QVector>
#include <QVector4D>
#include <QList>
#include <QtMath>

#include "KoColorConversions.h"
#include "KoColorSpace.h"
#include "KoColorDisplayRendererInterface.h"
#include "KoChannelInfo.h"
#include <KoColorModelStandardIds.h>
#include <QPointer>

#include "kis_debug.h"

struct KisVisualColorSelectorShape::Private
{
    QImage gradient;
    QImage alphaMask;
    bool imagesNeedUpdate { true };
    bool alphaNeedsUpdate { true };
    bool acceptTabletEvents { false };
    QPointF currentCoordinates; // somewhat redundant?
    QPointF dragStart;
    QVector4D currentChannelValues;
    Dimensions dimension;
    int channel1;
    int channel2;
};

KisVisualColorSelectorShape::KisVisualColorSelectorShape(KisVisualColorSelector *parent,
                                                         KisVisualColorSelectorShape::Dimensions dimension,
                                                         int channel1,
                                                         int channel2): QWidget(parent), m_d(new Private)
{
    m_d->dimension = dimension;
    int maxchannel = parent->selectorModel()->colorSpace()->colorChannelCount()-1;
    m_d->channel1 = qBound(0, channel1, maxchannel);
    m_d->channel2 = qBound(0, channel2, maxchannel);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

KisVisualColorSelectorShape::~KisVisualColorSelectorShape()
{
}

QPointF KisVisualColorSelectorShape::getCursorPosition() {
    return m_d->currentCoordinates;
}

void KisVisualColorSelectorShape::setCursorPosition(QPointF position, bool signal)
{
    QPointF newPos(qBound(0.0, position.x(), 1.0), qBound(0.0, position.y(), 1.0));
    if (newPos != m_d->currentCoordinates)
    {
        m_d->currentCoordinates = newPos;
        // for internal consistency, because we have a bit of redundancy here
        m_d->currentChannelValues[m_d->channel1] = newPos.x();
        if (m_d->dimension == Dimensions::twodimensional){
            m_d->currentChannelValues[m_d->channel2] = newPos.y();
        }
        update();
        if (signal){
            emit sigCursorMoved(newPos);
        }
    }
}

void KisVisualColorSelectorShape::setChannelValues(QVector4D channelValues, bool setCursor)
{
    //qDebug() << this  << "setChannelValues";
    m_d->currentChannelValues = channelValues;
    if (setCursor) {
        m_d->currentCoordinates = QPointF(qBound(0.f, channelValues[m_d->channel1], 1.f),
                                          qBound(0.f, channelValues[m_d->channel2], 1.f));
    }
    else {
        // for internal consistency, because we have a bit of redundancy here
        m_d->currentChannelValues[m_d->channel1] = m_d->currentCoordinates.x();
        if (m_d->dimension == Dimensions::twodimensional){
            m_d->currentChannelValues[m_d->channel2] = m_d->currentCoordinates.y();
        }
    }
    m_d->imagesNeedUpdate = true;
    update();
}

void KisVisualColorSelectorShape::setAcceptTabletEvents(bool on)
{
    m_d->acceptTabletEvents = on;
}

void KisVisualColorSelectorShape::forceImageUpdate()
{
    //qDebug() << this  << "forceImageUpdate";
    m_d->alphaNeedsUpdate = true;
    m_d->imagesNeedUpdate = true;
}

QColor KisVisualColorSelectorShape::getColorFromConverter(KoColor c)
{
    const KoColorDisplayRendererInterface *renderer = selectorModel()->displayRenderer();
    // Should we convert to painting color space first as Advanced Color Selector does?
    // KoColor color = c.convertedTo(renderer->getPaintingColorSpace());
    return renderer->toQColor(c);
}

KisVisualColorSelector *KisVisualColorSelectorShape::colorSelector() const
{
    KisVisualColorSelector* selectorWidget = qobject_cast<KisVisualColorSelector*>(parent());
    KIS_ASSERT(selectorWidget);
    return selectorWidget;
}

KisVisualColorModel *KisVisualColorSelectorShape::selectorModel() const
{
    KisVisualColorSelector* selectorWidget = qobject_cast<KisVisualColorSelector*>(parent());
    KIS_ASSERT(selectorWidget);
    return selectorWidget->selectorModel();
}

bool KisVisualColorSelectorShape::imagesNeedUpdate() const {
    return m_d->imagesNeedUpdate;
}

QImage KisVisualColorSelectorShape::getImageMap()
{
    //qDebug() << this  << ">>>>>>>>> getImageMap()" << m_d->imagesNeedUpdate;

    if (m_d->imagesNeedUpdate) {
        // Fill a buffer with the right kocolors
        m_d->gradient = renderBackground(m_d->currentChannelValues, selectorModel()->colorSpace()->pixelSize());
        m_d->imagesNeedUpdate = false;
    }
    return m_d->gradient;
}

const QImage KisVisualColorSelectorShape::getAlphaMask() const
{
    if (m_d->alphaNeedsUpdate) {
        m_d->alphaMask = renderAlphaMask();
        m_d->alphaNeedsUpdate = false;
    }
    return m_d->alphaMask;
}

QImage KisVisualColorSelectorShape::convertImageMap(const quint8 *rawColor, quint32 bufferSize, QSize imgSize) const
{
    const KoColorSpace *colorSpace = selectorModel()->colorSpace();
    Q_ASSERT(bufferSize == imgSize.width() * imgSize.height() * colorSpace->pixelSize());
    const KoColorDisplayRendererInterface *renderer = selectorModel()->displayRenderer();

    // Convert the buffer to a qimage
    QImage image = renderer->convertToQImage(colorSpace, rawColor, imgSize.width(), imgSize.height());

    // safeguard:
    if (image.isNull())
    {
        image = QImage(width(), height(), QImage::Format_ARGB32);
        image.fill(Qt::black);
    }

    return image;
}

QImage KisVisualColorSelectorShape::renderBackground(const QVector4D &channelValues, quint32 pixelSize) const
{
    const KisVisualColorModel *selector = selectorModel();
    Q_ASSERT(selector);

    // Hi-DPI aware rendering requires that we determine the device pixel dimension;
    // actual widget size in device pixels is not accessible unfortunately, it might be 1px smaller...
    const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const int deviceWidth = qCeil(width() * devicePixelRatioF());
    const int deviceHeight = qCeil(height() * devicePixelRatioF());
    quint32 imageSize = deviceWidth * deviceHeight * selector->colorSpace()->pixelSize();
    QScopedArrayPointer<quint8> raw(new quint8[imageSize] {});
    quint8 *dataPtr = raw.data();
    QVector4D coordinates = channelValues;

    QImage alpha = getAlphaMask();
    bool checkAlpha = !alpha.isNull() && alpha.valid(deviceWidth - 1, deviceHeight - 1);
    KIS_SAFE_ASSERT_RECOVER(!checkAlpha || alpha.format() == QImage::Format_Alpha8) {
        checkAlpha = false;
    }

    KoColor filler(Qt::white, selector->colorSpace());
    for (int y = 0; y < deviceHeight; y++) {
        const uchar *alphaLine = checkAlpha ? alpha.scanLine(y) : 0;
        for (int x=0; x < deviceWidth; x++) {
            if (!checkAlpha || alphaLine[x]) {
                QPointF newcoordinate = convertWidgetCoordinateToShapeCoordinate(QPointF(x, y) * deviceDivider);
                coordinates[m_d->channel1] = newcoordinate.x();
                if (m_d->dimension == Dimensions::twodimensional) {
                    coordinates[m_d->channel2] = newcoordinate.y();
                }
                KoColor c = selector->convertChannelValuesToKoColor(coordinates);
                memcpy(dataPtr, c.data(), pixelSize);
            }
            else {
                // need to write a color with non-zero alpha, otherwise the display converter
                // will for some arcane reason crop the final QImage and screw rendering
                memcpy(dataPtr, filler.data(), pixelSize);
            }
            dataPtr += pixelSize;
        }
    }
    QImage image = convertImageMap(raw.data(), imageSize, QSize(deviceWidth, deviceHeight));
    image.setDevicePixelRatio(devicePixelRatioF());

    if (!alpha.isNull()) {
        QPainter painter(&image);
        // transfer alphaMask to Alpha channel
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawImage(0, 0, alpha);
    }

    return image;
}

QImage KisVisualColorSelectorShape::renderAlphaMask() const
{
    return QImage();
}

QPointF KisVisualColorSelectorShape::mousePositionToShapeCoordinate(const QPointF &pos, const QPointF &dragStart) const
{
    Q_UNUSED(dragStart);
    return convertWidgetCoordinateToShapeCoordinate(pos);
}

void KisVisualColorSelectorShape::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_d->dragStart = e->localPos();
        QPointF coordinates = mousePositionToShapeCoordinate(e->localPos(), m_d->dragStart);
        setCursorPosition(coordinates, true);
    }
    else {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        QPointF coordinates = mousePositionToShapeCoordinate(e->localPos(), m_d->dragStart);
        setCursorPosition(coordinates, true);
    } else {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
    }
}

void KisVisualColorSelectorShape::tabletEvent(QTabletEvent* event)
{
    // only accept tablet events that are associated to "left" button
    // NOTE: QTabletEvent does not have a windowPos() equivalent, but we don't need it
    if (m_d->acceptTabletEvents &&
        (event->button() == Qt::LeftButton || (event->buttons() & Qt::LeftButton)))
    {
        event->accept();
        switch (event->type()) {
        case  QEvent::TabletPress: {
            QMouseEvent mouseEvent(QEvent::MouseButtonPress, event->posF(), event->posF(),
                                   event->globalPosF(), event->button(), event->buttons(),
                                   event->modifiers(), Qt::MouseEventSynthesizedByApplication);
            mousePressEvent(&mouseEvent);
            break;
        }
        case QEvent::TabletMove: {
            QMouseEvent mouseEvent(QEvent::MouseMove, event->posF(), event->posF(),
                                   event->globalPosF(), event->button(), event->buttons(),
                                   event->modifiers(), Qt::MouseEventSynthesizedByApplication);
            mouseMoveEvent(&mouseEvent);
            break;
        }
        case QEvent::TabletRelease: {
            QMouseEvent mouseEvent(QEvent::MouseButtonRelease, event->posF(), event->posF(),
                                   event->globalPosF(), event->button(), event->buttons(),
                                   event->modifiers(), Qt::MouseEventSynthesizedByApplication);
            mouseReleaseEvent(&mouseEvent);
            break;
        }
        default:
            event->ignore();
        }
    }
}

void KisVisualColorSelectorShape::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QImage fullSelector = getImageMap();
    painter.drawImage(0, 0, fullSelector);
    painter.setRenderHint(QPainter::Antialiasing);
    drawCursor(painter);
}

void KisVisualColorSelectorShape::resizeEvent(QResizeEvent *)
{
    forceImageUpdate();
    setMask(getMaskMap());
}

KisVisualColorSelectorShape::Dimensions KisVisualColorSelectorShape::getDimensions() const
{
    return m_d->dimension;
}

KoColor KisVisualColorSelectorShape::getCurrentColor()
{
    const KisVisualColorModel *selector = selectorModel();
    if (selector)
    {
        return selector->convertChannelValuesToKoColor(m_d->currentChannelValues);
    }
    return KoColor();
}

QVector <int> KisVisualColorSelectorShape::getChannels() const
{
    QVector <int> channels(2);
    channels[0] = m_d->channel1;
    channels[1] = m_d->channel2;
    return channels;
}

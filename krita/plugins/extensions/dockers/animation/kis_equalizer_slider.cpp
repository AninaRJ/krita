/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_equalizer_slider.h"

#include <QStyle>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QStyleOptionSlider>

#include "kis_global.h"
#include "kis_debug.h"

#include "timeline_color_scheme.h"


struct KisEqualizerSlider::Private
{
    Private(KisEqualizerSlider *_q) : q(_q), isRightmost(false) {}

    KisEqualizerSlider *q;
    bool isRightmost;


    QRect boundingRect() const;
    QRect sliderRect() const;

    int mousePosToValue(const QPoint &pt, bool round) const;
};

KisEqualizerSlider::KisEqualizerSlider(QWidget *parent)
    : QAbstractSlider(parent),
      m_d(new Private(this))
{
    setOrientation(Qt::Vertical);
    setFocusPolicy(Qt::WheelFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

KisEqualizerSlider::~KisEqualizerSlider()
{
}

void KisEqualizerSlider::setRightmost(bool value)
{
    m_d->isRightmost = value;
}

QRect KisEqualizerSlider::Private::boundingRect() const
{
    QRect bounds = q->rect().adjusted(0, 0, -isRightmost, -1);
    return bounds;
}

QRect KisEqualizerSlider::Private::sliderRect() const
{
    const int offset = 3;
    QRect filling = boundingRect().adjusted(offset + 1, offset + 1,
                                            -offset, -offset);

    return filling;
}

int KisEqualizerSlider::Private::mousePosToValue(const QPoint &pt,  bool round) const
{
    const QRect areaRect = sliderRect();

    int rawValue = -pt.y() + (areaRect.top() + areaRect.height());
    int maxRawValue = areaRect.height();

    int value = QStyle::sliderValueFromPosition(q->minimum(), q->maximum(), rawValue, maxRawValue);

    if (round) {
        const int singleStep = q->singleStep();
        value = ((value + singleStep / 2) / singleStep) * singleStep;
    }

    return value;
}

void KisEqualizerSlider::mousePressEvent(QMouseEvent *ev)
{
    if (maximum() == minimum() || (ev->buttons() ^ ev->button())) {
        ev->ignore();
        return;
    }

    const bool precise = ev->modifiers() & Qt::ControlModifier ||
        ev->button() == Qt::RightButton;

    int value = m_d->mousePosToValue(ev->pos(), !precise);
    setSliderPosition(value);
    triggerAction(SliderMove);
    setRepeatAction(SliderNoAction);
}

void KisEqualizerSlider::mouseMoveEvent(QMouseEvent *ev)
{
    if (ev->modifiers() & Qt::ShiftModifier &&
        !rect().contains(ev->pos())) {

        ev->ignore();
        return;
    }

    const bool precise = ev->modifiers() & Qt::ControlModifier ||
        ev->buttons() & Qt::RightButton;

    int value = m_d->mousePosToValue(ev->pos(), !precise);
    setSliderPosition(value);
    triggerAction(SliderMove);
    setRepeatAction(SliderNoAction);
}

void KisEqualizerSlider::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev);
}

QSize KisEqualizerSlider::sizeHint() const
{
    return QSize(25, 150);
}

QSize KisEqualizerSlider::minimumSizeHint() const
{
    return QSize(10, 100);
}

void KisEqualizerSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const QRect bounds = m_d->boundingRect();
    const QColor backgroundColor = palette().color(QPalette::Base);


    QPainter p(this);

    { // draw border

        QStyleOptionViewItemV4 option; // empty!
        const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
        const QColor gridColor = static_cast<QRgb>(gridHint);
        const QPen gridPen(gridColor);

        p.setPen(gridPen);
        p.setBrush(backgroundColor);
        p.drawRect(bounds);
    }

    { // draw slider
        QRect sliderRect = m_d->sliderRect();
        const int sliderPos =  QStyle::sliderPositionFromValue(minimum(), maximum(), value(), sliderRect.height());
        sliderRect.adjust(0, sliderRect.height() - sliderPos, 0, 0);

        p.setPen(Qt::transparent);
        p.setBrush(TimelineColorScheme::instance()->onionSkinsSliderColor());

        p.drawRect(sliderRect);
    }

    QString textValue = QString::number(value());

    { // draw text
        QPalette::ColorRole textRole = QPalette::Text;

        //Draw text shadow, This will increase readability when the background of same color
        QRect shadowRect(bounds);
        shadowRect.translate(1,1);
        QColor textColor = palette().color(textRole);
        QColor shadowColor = (textColor.value() <= 128)
            ? QColor(255,255,255,160) : QColor(0,0,0,160);

        p.setPen(shadowColor);
        p.drawText(shadowRect, Qt::AlignCenter, textValue);

        p.setPen(textColor);
        p.drawText(bounds, Qt::AlignCenter, textValue);
    }

    // draw focus rect
    if (hasFocus()) {
        QStyleOptionFocusRect fropt;
        fropt.initFrom(this);
        fropt.backgroundColor = backgroundColor;

        int dfw1 = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &fropt, this) + 1,
            dfw2 = dfw1 * 2;
        fropt.rect = kisGrowRect(bounds, -dfw1 - dfw2);

        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &fropt, &p, this);
    }
}

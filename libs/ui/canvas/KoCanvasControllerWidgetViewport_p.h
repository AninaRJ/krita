/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOCANVASCONTROLLERWIDGETVIEWPORT_P_H
#define KOCANVASCONTROLLERWIDGETVIEWPORT_P_H

#include <QWidget>
#include <QSize>
#include <QPoint>

class KoCanvasControllerWidget;
class KoShape;

class Viewport : public QWidget
{
    Q_OBJECT

public:

    explicit Viewport(KoCanvasControllerWidget *parent);
    ~Viewport() override {}

    void setCanvas(QWidget *canvas);
    QWidget *canvas() const {
        return m_canvas;
    }

    /**
     * Resizes the subordinate canvas widget as needed
     */
     void resetLayout();

public:

    void handleDragEnterEvent(QDragEnterEvent *event);
    void handleDropEvent(QDropEvent *event);
    void handleDragMoveEvent(QDragMoveEvent *event);
    void handleDragLeaveEvent(QDragLeaveEvent *event);
    void handlePaintEvent(QPainter &gc, QPaintEvent *event);

private:

    QPointF correctPosition(const QPoint &point) const;
    void repaint(KoShape *shape);



private:

    KoCanvasControllerWidget *m_parent;
    KoShape *m_draggedShape;
    QWidget *m_canvas;
};

#endif

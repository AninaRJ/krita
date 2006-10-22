/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_OPENGL_IMAGE_CONTEXT_H_
#define KIS_OPENGL_IMAGE_CONTEXT_H_

#include <config.h>
#include <config-krita.h>

#ifdef HAVE_OPENGL

#include <map>

#include <qgl.h>
#include <QObject>
#include <q3valuevector.h>

#include <krita_export.h>

#include "kis_shared.h"
#include "kis_types.h"

class QRegion;

// XXX: Qt 4.1 provides a QGLContext class that may be relevant here

class KisOpenGLImageContext;
typedef KisSharedPtr<KisOpenGLImageContext> KisOpenGLImageContextSP;
class KoColorSpace;

class KRITAUI_EXPORT KisOpenGLImageContext : public QObject , public KisShared {

    Q_OBJECT

public:
    static KisOpenGLImageContextSP getImageContext(KisImageSP image, KoColorProfile *monitorProfile);

    KisOpenGLImageContext();
    virtual ~KisOpenGLImageContext();

public:
    // In order to use the image textures, the caller must pass
    // the sharedContextWidget() as the shareWidget argument to the
    // QGLWidget constructor.
    QGLWidget *sharedContextWidget() const;

    void setMonitorProfile(KoColorProfile *profile);
    void setHDRExposure(float exposure);

    GLuint backgroundTexture() const;

    static const int BACKGROUND_TEXTURE_WIDTH = 32;
    static const int BACKGROUND_TEXTURE_HEIGHT = 32;

    // Get the image texture tile containing the point (pixelX, pixelY).
    GLuint imageTextureTile(int pixelX, int pixelY) const;

    int imageTextureTileWidth() const;
    int imageTextureTileHeight() const;

    /**
     * Select selection visualization rendering.
     *
     * @param enable Set to true to enable selection visualization rendering.
     */
    void setSelectionDisplayEnabled(bool enable);

    /**
     * Update the image textures for the given image rectangle.
     *
     * @param imageRect The rectangle to update in image coordinates.
     */
    void update(const QRect& imageRect);

signals:
    /**
     * Clients using the KisOpenGLImageContext should connect to the
     * following signals rather than to the KisImage's own equivalent
     * signals. This ensures that the image textures are always up to date
     * when used.
     */

    /**
     * Emitted whenever an action has caused the image to be recomposited.
     *
     * @param rc     The rect that has been recomposited.
     */
    void sigImageUpdated(QRect rc);

    /**
     * Emitted whenever the image size changes.
     *
     * @param width  New image width
     * @param height New image height
     */
    void sigSizeChanged(qint32 width, qint32 height);

protected:
    KisOpenGLImageContext(KisImageSP image, KoColorProfile *monitorProfile);

    void generateBackgroundTexture();
    void createImageTextureTiles();
    void destroyImageTextureTiles();
    int imageTextureTileIndex(int x, int y) const;
    void updateImageTextureTiles(const QRect& rect);

    static KoColorSpace* textureColorSpaceForImageColorSpace(KoColorSpace *imageColorSpace);
    static bool imageCanShareImageContext(KisImageSP image);

protected slots:
    void slotImageUpdated(QRect r);
    void slotImageSizeChanged(qint32 w, qint32 h);

private:
    KisImageSP m_image;
    KoColorProfile *m_monitorProfile;
    float m_exposure;
    bool m_displaySelection;

    GLuint m_backgroundTexture;

    static const int PREFERRED_IMAGE_TEXTURE_WIDTH;
    static const int PREFERRED_IMAGE_TEXTURE_HEIGHT;

    Q3ValueVector<GLuint> m_imageTextureTiles;
    int m_imageTextureTileWidth;
    int m_imageTextureTileHeight;
    int m_numImageTextureTileColumns;

    // We create a single OpenGL context and share it between all views
    // in the process. Apparently with some OpenGL implementations, only
    // one context will be hardware accelerated.
    static QGLWidget *SharedContextWidget;
    static int SharedContextWidgetRefCount;

    typedef std::map<KisImageSP, KisOpenGLImageContext*> ImageContextMap;

    static ImageContextMap imageContextMap;
};

#endif // HAVE_OPENGL

#endif // KIS_OPENGL_IMAGE_CONTEXT_H_


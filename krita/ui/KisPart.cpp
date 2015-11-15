/* This file is part of the KDE project
 * Copyright (C) 1998-1999 Torben Weis       <weis@kde.org>
 * Copyright (C) 2000-2005 David Faure       <faure@kde.org>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010-2012 Boudewijn Rempt   <boud@kogmbh.com>
 * Copyright (C) 2011 Inge Wallin            <ingwa@kogmbh.com>
 * Copyright (C) 2015 Michael Abrahams       <miabraha@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisPart.h"

#include "KoProgressProxy.h"
#include <KoCanvasController.h>
#include <KoCanvasControllerWidget.h>
#include <KoColorSpaceEngine.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoShapeBasedDocumentBase.h>
#include <KoResourceServerProvider.h>
#include <kis_icon.h>

#include "KisApplication.h"
#include "KisDocument.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "KisOpenPane.h"
#include "KisImportExportManager.h"

#include <kis_debug.h>
#include <KoResourcePaths.h>
#include <KoDialog.h>
#include <kdesktopfile.h>
#include <QMessageBox>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <QKeySequence>

#include <QDialog>
#include <QGraphicsScene>
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QGlobalStatic>

#include "KisView.h"
#include "KisDocument.h"
#include "kis_config.h"
#include "kis_clipboard.h"
#include "kis_custom_image_widget.h"
#include "kis_image_from_clipboard_widget.h"
#include "kis_shape_controller.h"
#include "kis_resource_server_provider.h"
#include "kis_animation_cache_populator.h"
#include "kis_idle_watcher.h"
#include "kis_image.h"
#include "KisImportExportManager.h"
#include "KisDocumentEntry.h"
#include "KoToolManager.h"

#include "kis_color_manager.h"
#include "kis_debug.h"

#include "kis_action.h"
#include "kis_action_registry.h"

Q_GLOBAL_STATIC(KisPart, s_instance)


class Q_DECL_HIDDEN KisPart::Private
{
public:
    Private(KisPart *_part)
        : part(_part)
        , idleWatcher(2500)
        , animationCachePopulator(_part)
    {
    }

    ~Private()
    {
        delete canvasItem;
    }

    KisPart *part;

    QList<QPointer<KisView> > views;
    QList<QPointer<KisMainWindow> > mainWindows;
    QList<QPointer<KisDocument> > documents;
    QString templatesResourcePath;

    QGraphicsItem *canvasItem{0};
    KisOpenPane *startupWidget{0};
    KActionCollection *actionCollection{0};

    KisIdleWatcher idleWatcher;
    KisAnimationCachePopulator animationCachePopulator;

    void loadActions();
};

// Basically, we are going to insert the current UI/MainWindow ActionCollection
// into the KisActionRegistry.
void KisPart::Private::loadActions()
{
    actionCollection = part->currentMainwindow()->viewManager()->actionCollection();

    KisActionRegistry * actionRegistry = KisActionRegistry::instance();

    foreach (auto action, actionCollection->actions()) {
        auto name = action->objectName();
        actionRegistry->addAction(action->objectName(), action);
    }
};

KisPart* KisPart::instance()
{
    return s_instance;
}


KisPart::KisPart()
    : d(new Private(this))
{
    setTemplatesResourcePath(QLatin1String("krita/templates/"));

    // Preload all the resources in the background
    Q_UNUSED(KoResourceServerProvider::instance());
    Q_UNUSED(KisResourceServerProvider::instance());
    Q_UNUSED(KisColorManager::instance());

    connect(this, SIGNAL(documentOpened(QString)),
            this, SLOT(updateIdleWatcherConnections()));

    connect(this, SIGNAL(documentClosed(QString)),
            this, SLOT(updateIdleWatcherConnections()));

    connect(&d->idleWatcher, SIGNAL(startedIdleMode()),
            &d->animationCachePopulator, SLOT(slotRequestRegeneration()));

    d->animationCachePopulator.slotRequestRegeneration();
}

KisPart::~KisPart()
{
    while (!d->documents.isEmpty()) {
        delete d->documents.takeFirst();
    }

    while (!d->views.isEmpty()) {
        delete d->views.takeFirst();
    }

    while (!d->mainWindows.isEmpty()) {
        delete d->mainWindows.takeFirst();
    }

    delete d;
}

void KisPart::updateIdleWatcherConnections()
{
    QVector<KisImageSP> images;

    foreach (QPointer<KisDocument> document, documents()) {
        images << document->image();
    }

    d->idleWatcher.setTrackedImages(images);
}

void KisPart::addDocument(KisDocument *document)
{
    //dbgUI << "Adding document to part list" << document;
    Q_ASSERT(document);
    if (!d->documents.contains(document)) {
        d->documents.append(document);
        emit documentOpened('/'+objectName());
    }
}

QList<QPointer<KisDocument> > KisPart::documents() const
{
    return d->documents;
}

KisDocument *KisPart::createDocument() const
{
    KisDocument *doc = new KisDocument();
    return doc;
}


int KisPart::documentCount() const
{
    return d->documents.size();
}

void KisPart::removeDocument(KisDocument *document)
{
    d->documents.removeAll(document);
    emit documentClosed('/'+objectName());
    document->deleteLater();
}

KisMainWindow *KisPart::createMainWindow()
{
    KisMainWindow *mw = new KisMainWindow();

    dbgUI <<"mainWindow" << (void*)mw << "added to view" << this;
    d->mainWindows.append(mw);

    return mw;
}

KisView *KisPart::createView(KisDocument *document,
                             KoCanvasResourceManager *resourceManager,
                             KActionCollection *actionCollection,
                             QWidget *parent)
{
    // If creating the canvas fails, record this and disable OpenGL next time
    KisConfig cfg;
    KConfigGroup grp( KSharedConfig::openConfig(), "krita/crashprevention");
    if (grp.readEntry("CreatingCanvas", false)) {
        cfg.setUseOpenGL(false);
    }
    if (cfg.canvasState() == "OPENGL_FAILED") {
        cfg.setUseOpenGL(false);
    }
    grp.writeEntry("CreatingCanvas", true);
    grp.sync();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    KisView *view  = new KisView(document, resourceManager, actionCollection, parent);
    QApplication::restoreOverrideCursor();

    // Record successful canvas creation
    grp.writeEntry("CreatingCanvas", false);
    grp.sync();

    addView(view);

    return view;
}

void KisPart::addView(KisView *view)
{
    if (!view)
        return;

    if (!d->views.contains(view)) {
        d->views.append(view);
    }

    connect(view, SIGNAL(destroyed()), this, SLOT(viewDestroyed()));

    emit sigViewAdded(view);
}

void KisPart::removeView(KisView *view)
{
    if (!view) return;

    emit sigViewRemoved(view);

    QPointer<KisDocument> doc = view->document();
    d->views.removeAll(view);

    if (doc) {
        bool found = false;
        foreach(QPointer<KisView> view, d->views) {
            if (view && view->document() == doc) {
                found = true;
                break;
            }
        }
        if (!found) {
            removeDocument(doc);
        }
    }
}

QList<QPointer<KisView> > KisPart::views() const
{
    return d->views;
}

int KisPart::viewCount(KisDocument *doc) const
{
    if (!doc) {
        return d->views.count();
    }
    else {
        int count = 0;
        foreach(QPointer<KisView> view, d->views) {
            if (view->document() == doc) {
                count++;
            }
        }
        return count;
    }
}

QGraphicsItem *KisPart::canvasItem(KisDocument *document, bool create)
{
    if (create && !d->canvasItem) {
        d->canvasItem = createCanvasItem(document);
    }
    return d->canvasItem;
}

QGraphicsItem *KisPart::createCanvasItem(KisDocument *document)
{
    if (!document) return 0;

    KisView *view = createView(document, 0, 0, 0);
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    QWidget *canvasController = view->findChild<KoCanvasControllerWidget*>();
    proxy->setWidget(canvasController);
    return proxy;
}

void KisPart::removeMainWindow(KisMainWindow *mainWindow)
{
    dbgUI <<"mainWindow" << (void*)mainWindow <<"removed from doc" << this;
    if (mainWindow) {
        d->mainWindows.removeAll(mainWindow);
    }
}

const QList<QPointer<KisMainWindow> > &KisPart::mainWindows() const
{
    return d->mainWindows;
}

int KisPart::mainwindowCount() const
{
    return d->mainWindows.count();
}


KisMainWindow *KisPart::currentMainwindow() const
{
    QWidget *widget = qApp->activeWindow();
    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(widget);
    while (!mainWindow && widget) {
        widget = widget->parentWidget();
        mainWindow = qobject_cast<KisMainWindow*>(widget);
    }

    if (!mainWindow && mainWindows().size() > 0) {
        mainWindow = mainWindows().first();
    }
    return mainWindow;

}

KisIdleWatcher* KisPart::idleWatcher() const
{
    return &d->idleWatcher;
}

KisAnimationCachePopulator* KisPart::cachePopulator() const
{
    return &d->animationCachePopulator;
}

void KisPart::openExistingFile(const QUrl &url)
{
    Q_ASSERT(url.isLocalFile());
    qApp->setOverrideCursor(Qt::BusyCursor);
    KisDocument *document = createDocument();
    if (!document->openUrl(url)) {
        delete document;
        return;
    }
    if (!document->image()) {
        delete document;
        return;
    }
    document->setModified(false);
    addDocument(document);

    KisMainWindow *mw = 0;
    if (d->startupWidget) {
        mw = qobject_cast<KisMainWindow*>(d->startupWidget->parent());
    }
    if (!mw) {
        mw = currentMainwindow();
    }

    mw->addViewAndNotifyLoadingCompleted(document);

    if (d->startupWidget) {
        d->startupWidget->setParent(0);
        d->startupWidget->hide();
    }
    qApp->restoreOverrideCursor();
}

void KisPart::configureShortcuts()
{
    d->loadActions();

    auto actionRegistry = KisActionRegistry::instance();
    actionRegistry->configureShortcuts(d->actionCollection);

    // Update the non-UI actions.  That includes:
    //  - Shortcuts called inside of tools
    //  - Perhaps other things?
    KoToolManager::instance()->updateToolShortcuts();

    // Now update the UI actions.
    foreach(KisMainWindow *mainWindow, d->mainWindows) {
        KActionCollection *ac = mainWindow->actionCollection();

        ac->updateShortcuts();

        // Loop through mainWindow->actionCollections() to modify tooltips
        // so that they list shortcuts at the end in parentheses
        foreach( QAction* action, ac->actions())
        {
            // Remove any existing suffixes from the tooltips.
            // Note this regexp starts with a space, e.g. " (Ctrl-a)"
            QString strippedTooltip = action->toolTip().remove(QRegExp("\\s\\(.*\\)"));

            // Now update the tooltips with the new shortcut info.
            if(action->shortcut() == QKeySequence(0))
                 action->setToolTip(strippedTooltip);
            else
                 action->setToolTip( strippedTooltip + " (" + action->shortcut().toString() + ")");
        }
    }
}

void KisPart::openTemplate(const QUrl &url)
{
    qApp->setOverrideCursor(Qt::BusyCursor);
    KisDocument *document = createDocument();

    bool ok = document->loadNativeFormat(url.toLocalFile());
    document->setModified(false);
    document->undoStack()->clear();

    if (ok) {
        QMimeDatabase db;
        QString mimeType = db.mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension).name();
        // in case this is a open document template remove the -template from the end
        mimeType.remove( QRegExp( "-template$" ) );
        document->setMimeTypeAfterLoading(mimeType);
        document->resetURL();
        document->setEmpty();
    } else {
        document->showLoadingErrorDialog();
        document->initEmpty();
    }
    addDocument(document);

    KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->startupWidget->parent());
    if (!mw) mw = currentMainwindow();
    mw->addViewAndNotifyLoadingCompleted(document);

    d->startupWidget->setParent(0);
    d->startupWidget->hide();
    qApp->restoreOverrideCursor();
}

void KisPart::viewDestroyed()
{
    KisView *view = qobject_cast<KisView*>(sender());
    if (view) {
        removeView(view);
    }
}

void KisPart::addRecentURLToAllMainWindows(QUrl url)
{
    // Add to recent actions list in our mainWindows
    foreach(KisMainWindow *mainWindow, d->mainWindows) {
        mainWindow->addRecentURL(url);
    }
}

void KisPart::showStartUpWidget(KisMainWindow *mainWindow, bool alwaysShow)
{

#ifndef NDEBUG
    if (d->templatesResourcePath.isEmpty())
        dbgUI << "showStartUpWidget called, but setTemplatesResourcePath() never called. This will not show a lot";
#endif

    if (!alwaysShow) {
        KConfigGroup cfgGrp( KSharedConfig::openConfig(), "TemplateChooserDialog");
        QString fullTemplateName = cfgGrp.readPathEntry("AlwaysUseTemplate", QString());
        if (!fullTemplateName.isEmpty()) {
            QUrl url(fullTemplateName);
            QFileInfo fi(url.toLocalFile());
            if (!fi.exists()) {
                const QString templatesResourcePath = this->templatesResourcePath();
                QString desktopfile = KoResourcePaths::findResource("data", templatesResourcePath + "*/" + fullTemplateName);
                if (desktopfile.isEmpty()) {
                    desktopfile = KoResourcePaths::findResource("data", templatesResourcePath + fullTemplateName);
                }
                if (desktopfile.isEmpty()) {
                    fullTemplateName.clear();
                } else {
                    KDesktopFile f(desktopfile);
                    fullTemplateName = QFileInfo(desktopfile).absolutePath() + '/' + f.readUrl();
                }
            }
            if (!fullTemplateName.isEmpty()) {
                openTemplate(QUrl::fromLocalFile(fullTemplateName));
                return;
            }
        }
    }

    if (d->startupWidget) {
        delete d->startupWidget;
    }
    const QStringList mimeFilter = KisImportExportManager::mimeFilter(KIS_MIME_TYPE,
                                                                      KisImportExportManager::Import,
                                                                      KisDocumentEntry::extraNativeMimeTypes());

    d->startupWidget = new KisOpenPane(0, mimeFilter, d->templatesResourcePath);
    d->startupWidget->setWindowModality(Qt::WindowModal);
    QList<CustomDocumentWidgetItem> widgetList = createCustomDocumentWidgets(d->startupWidget);
    foreach(const CustomDocumentWidgetItem & item, widgetList) {
        d->startupWidget->addCustomDocumentWidget(item.widget, item.title, item.icon);
        connect(item.widget, SIGNAL(documentSelected(KisDocument*)), this, SLOT(startCustomDocument(KisDocument*)));
    }

    connect(d->startupWidget, SIGNAL(openExistingFile(const QUrl&)), this, SLOT(openExistingFile(const QUrl&)));
    connect(d->startupWidget, SIGNAL(openTemplate(const QUrl&)), this, SLOT(openTemplate(const QUrl&)));

    d->startupWidget->setParent(mainWindow);
    d->startupWidget->setWindowFlags(Qt::Dialog);
    d->startupWidget->exec();
}

QList<KisPart::CustomDocumentWidgetItem> KisPart::createCustomDocumentWidgets(QWidget * parent)
{
    KisConfig cfg;

    int w = cfg.defImageWidth();
    int h = cfg.defImageHeight();
    const double resolution = cfg.defImageResolution();
    const QString colorModel = cfg.defColorModel();
    const QString colorDepth = cfg.defaultColorDepth();
    const QString colorProfile = cfg.defColorProfile();

    QList<KisPart::CustomDocumentWidgetItem> widgetList;
    {
        KisPart::CustomDocumentWidgetItem item;
        item.widget = new KisCustomImageWidget(parent,
                                               w,
                                               h,
                                               resolution,
                                               colorModel,
                                               colorDepth,
                                               colorProfile,
                                               i18n("Unnamed"));

        item.icon = "application-x-krita";
        widgetList << item;
    }

    {
        QSize sz = KisClipboard::instance()->clipSize();
        if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
            w = sz.width();
            h = sz.height();
        }

        KisPart::CustomDocumentWidgetItem item;
        item.widget = new KisImageFromClipboard(parent,
                                                w,
                                                h,
                                                resolution,
                                                colorModel,
                                                colorDepth,
                                                colorProfile,
                                                i18n("Unnamed"));

        item.title = i18n("Create from Clipboard");
        item.icon = "klipper";

        widgetList << item;

    }

    return widgetList;
}

void KisPart::setTemplatesResourcePath(const QString &templatesResourcePath)
{
    Q_ASSERT(!templatesResourcePath.isEmpty());
    Q_ASSERT(templatesResourcePath.endsWith(QLatin1Char('/')));

    d->templatesResourcePath = templatesResourcePath;
}

QString KisPart::templatesResourcePath() const
{
    return d->templatesResourcePath;
}

void KisPart::startCustomDocument(KisDocument* doc)
{
    addDocument(doc);
    KisMainWindow *mw = qobject_cast<KisMainWindow*>(d->startupWidget->parent());
    if (!mw) mw = currentMainwindow();

    mw->addViewAndNotifyLoadingCompleted(doc);

    d->startupWidget->setParent(0);
    d->startupWidget->hide();
}

KisInputManager* KisPart::currentInputManager()
{
    return instance()->currentMainwindow()->viewManager()->inputManager();
}


#include <KisPart.moc>
#include <QMimeDatabase>
#include <QMimeType>

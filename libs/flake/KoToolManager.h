/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2006, 2008 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KO_TOOL_MANAGER
#define KO_TOOL_MANAGER

#include "KoInputDevice.h"
#include "kritaflake_export.h"
#include "KoDerivedResourceConverter.h"
#include "KoAbstractCanvasResourceInterface.h"

#include <QObject>
#include <QList>

class KoCanvasController;
class KoShapeControllerBase;
class KoToolFactoryBase;
class KoCanvasBase;
class KoToolBase;
class KisKActionCollection;
class KoShape;
class KoShapeLayer;
class QKeySequence;

class QCursor;

/**
 * This class serves as a QAction-like control object for activation of a tool.
 *
 * It allows to implement a custom UI to control the activation of tools.
 * See KoToolBox & KoModeBox in the kowidgets library.
 *
 * KoToolAction objects are owned by the KoToolManager singleton
 * and live until the end of its lifetime.
 */
class KRITAFLAKE_EXPORT KoToolAction : public QObject
{
    Q_OBJECT
public:
    explicit KoToolAction(KoToolFactoryBase *toolFactory);
    ~KoToolAction() override;

public:
    QString id() const;             ///< The id of the tool
    QString iconText() const;       ///< The icontext of the tool
    QString toolTip() const;        ///< The tooltip of the tool
    QString iconName() const;       ///< The icon name of the tool
    QKeySequence shortcut() const;     ///< The shortcut to activate the tool

    QString section() const;        ///< The section the tool wants to be in.
    int priority() const;           ///< Lower number (higher priority) means coming first in the section.
    int buttonGroupId() const;      ///< A unique ID for this tool as passed by changedTool(), >= 0
    QString visibilityCode() const; ///< This tool should become visible when we Q_EMIT this string in toolCodesSelected()

    KoToolFactoryBase *toolFactory() const; ///< Factory to create new tool object instances

public Q_SLOTS:
    void trigger();                 ///< Request the activation of the tool

Q_SIGNALS:
    void changed();                 ///< Emitted when a property changes (shortcut ATM)

private:
    class Private;
    Private *const d;
};


/**
 * This class manages the activation and deactivation of tools for
 * each input device.
 *
 * Managing the active tool and switching tool based on various variables.
 *
 * The state of the toolbox will be the same for all views in the process so practically
 * you can say we have one toolbox per application instance (process).  Implementation
 * does not allow one widget to be in more then one view, so we just make sure the toolbox
 * is hidden in not-in-focus views.
 *
 * The ToolManager is a singleton and will manage all views in all applications that
 * are loaded in this process. This means you will have to register and unregister your view.
 * When creating your new view you should use a KoCanvasController() and register that
 * with the ToolManager like this:
@code
    MyGuiWidget::MyGuiWidget() {
        m_canvasController = new KoCanvasController(this);
        m_canvasController->setCanvas(m_canvas);
        KoToolManager::instance()->addControllers(m_canvasController));
    }
    MyGuiWidget::~MyGuiWidget() {
        KoToolManager::instance()->removeCanvasController(m_canvasController);
    }
@endcode
 *
 * For a new view that extends KoView all you need to do is implement KoView::createToolBox()
 *
 * KoToolManager also keeps track of the current tool based on a
   complex set of conditions and heuristics:

   - there is one active tool per KoCanvasController (and there is one KoCanvasController
     per view, because this is a class with scrollbars and a zoomlevel and so on)
   - for every pointing device (determined by the unique id of tablet,
     or 0 for mice -- you may have more than one mouse attached, but
     Qt cannot distinguish between them, there is an associated tool.
   - depending on things like tablet leave/enter proximity, incoming
     mouse or tablet events and a little timer (that gets stopped when
     we know what is what), the active pointing device is determined,
     and the active tool is set accordingly.

   Nota bene: if you use KoToolManager and register your canvases with
   it you no longer have to manually implement methods to route mouse,
   tablet, key or wheel events to the active tool. In fact, it's no
   longer interesting to you which tool is active; you can safely
   route the paint event through KoToolProxy::paint().

   (The reason the input events are handled completely by the
   toolmanager and the paint events not is that, generally speaking,
   it's okay if the tools get the input events first, but you want to
   paint your shapes or other canvas stuff first and only then paint
   the tool stuff.)

 */
class KRITAFLAKE_EXPORT KoToolManager : public QObject
{
    Q_OBJECT

public:
    KoToolManager();
    /// Return the toolmanager singleton
    static KoToolManager* instance();
    ~KoToolManager() override;

    /**
     * Initialize actions for switching to tools. The actions will have the
     * text / shortcut as stated by the toolFactory. If the application calls
     * this in their KoView extending class they will have all the benefits
     * from allowing this in the menus and to allow the use to configure the shortcuts used.
     */
    void initializeToolActions();

    /**
     * Register a new canvas controller
     * @param controller the view controller that this toolmanager will manage the tools for
     */
    void addController(KoCanvasController *controller);

    /**
     * Remove a set of controllers
     * When the controller is no longer used it should be removed so all tools can be
     * deleted and stop eating memory.
     * @param controller the controller that is removed
     */
    void removeCanvasController(KoCanvasController *controller);
    /**
     * Attempt to remove a controller.
     * This is automatically called when a controller's proxy object is deleted, and
     * it ensures that the controller is, in fact, removed, even if the creator forgot
     * to do so.
     * @param controller the proxy object of the controller to be removed
     */
    Q_SLOT void attemptCanvasControllerRemoval(QObject *controller);

    /// @return the active canvas controller
    KoCanvasController *activeCanvasController() const;

    /**
     * Returns the tool for the given tool id. The tool may be 0
     * @param canvas the canvas that is a child of a previously registered controller
     *    who's tool you want.
     * @param id the tool identifier
     * @see addController()
     */
    KoToolBase *toolById(KoCanvasBase *canvas, const QString &id) const;

    /// @return the currently active pointing device
    KoInputDevice currentInputDevice() const;

    /**
     * For the list of shapes find out which tool is the highest priority tool that can handle it.
     * @returns the toolId for the shapes.
     * @param shapes a list of shapes, a selection for example, that is used to look for the tool.
     */
    QString preferredToolForSelection(const QList<KoShape*> &shapes);

    /**
     * Returns the list of toolActions for the current tools.
     * @returns lists of toolActions for the current tools.
     */
    QList<KoToolAction*> toolActionList() const;

    /// Request tool activation for the given canvas controller
    void requestToolActivation(KoCanvasController *controller);

    /// Returns the toolId of the currently active tool
    QString activeToolId() const;

    void initializeCurrentToolForCanvas();

    void setConverter(KoDerivedResourceConverterSP converter, KoToolBase *tool);
    void setAbstractResource(KoAbstractCanvasResourceInterfaceSP abstractResource, KoToolBase *tool);

    class Private;
    /**
     * \internal return the private object for the toolmanager.
     */
    KoToolManager::Private *priv();

public Q_SLOTS:
    /**
     * Request switching tool
     * @param id the id of the tool
     */
    void switchToolRequested(const QString &id);

    /**
     * Request change input device
     * @param id the id of the input device
     */
    void switchInputDeviceRequested(const KoInputDevice &id);

    /**
     * Switches to the last tool used just before the current one, if any.
     */
    void switchBackRequested();

    /**
     * Notify theme changes
     */
    void themeChanged();
    
Q_SIGNALS:
    /**
     * Emitted when a new tool is going to override the current tool
     * @param canvas the currently active canvas.
     */
    void aboutToChangeTool(KoCanvasController *canvas);

    /**
     * Emitted when a new tool was selected or became active.
     * @param canvas the currently active canvas.
     */
    void changedTool(KoCanvasController *canvas);

    /**
     * Emitted after the selection changed to state which unique shape-types are now
     * in the selection.
     * @param types a list of string that are the shape types of the selected objects.
     */
    void toolCodesSelected(const QList<QString> &types);

    /**
     * Emitted after the current layer changed either its properties or to a new layer.
     * @param canvas the currently active canvas.
     * @param layer the layer that is selected.
     */
    void currentLayerChanged(const KoCanvasController *canvas, const KoShapeLayer *layer);

    /**
     * Every time a new input device gets used by a tool, this event is emitted.
     * @param device the new input device that the user picked up.
     */
    void inputDeviceChanged(const KoInputDevice &device);

    /**
     * Emitted whenever the active canvas changed.
     * @param canvas the new activated canvas (might be 0)
     */
    void changedCanvas(const KoCanvasBase *canvas);

    /**
     * Emitted whenever the active tool changes the status text.
     * @param statusText the new status text
     */
    void changedStatusText(const QString &statusText);

    /**
     * emitted whenever a new tool is dynamically added for the given canvas
     */
    void addedTool(KoToolAction *toolAction, KoCanvasController *canvas);

    /**
     * Emit the new tool option widgets to be used with this canvas.
     */
    void toolOptionWidgetsChanged(KoCanvasController *controller, const QList<QPointer<QWidget> > &widgets);

    /**
     * Emitted when the tool's text mode has changed.
     * @param inTextMode whether it is now in text mode.
     */
    void textModeChanged(bool text);

    /**
     * Emitted to create and store the opacity resource in \p tool. The opacity
     * is a derived or abstract resource depending on \p isOpacityPresetMode.
     */
    void createOpacityResource(bool isOpacityPresetMode, KoToolBase *tool);

private:
    KoToolManager(const KoToolManager&);
    KoToolManager operator=(const KoToolManager&);

    Q_PRIVATE_SLOT(d, void detachCanvas(KoCanvasController *controller))
    Q_PRIVATE_SLOT(d, void attachCanvas(KoCanvasController *controller))
    Q_PRIVATE_SLOT(d, void movedFocus(QWidget *from, QWidget *to))
    Q_PRIVATE_SLOT(d, void updateCursor(const QCursor &cursor))
    Q_PRIVATE_SLOT(d, void selectionChanged(const QList<KoShape*> &shapes))
    Q_PRIVATE_SLOT(d, void currentLayerChanged(const KoShapeLayer *layer))

    Private *const d;
};

#endif

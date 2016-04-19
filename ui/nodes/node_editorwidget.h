/****************************************************************************
**
 **Copyright (C) 2014
**
 **This file is generated by the Magus toolkit
**
 **THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 **"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 **LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 **A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 **OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 **SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 **LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 **DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 **THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 **(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 **OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#pragma once

#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QWidget>

class ObjectNodeItem;
class QtConnection;
class QtCompoundNode;
class QtNode;
class QtNodeGraphicsScene;
class QtPort;

enum {
	EDITOR_MODE_SCENE  = 0,
	EDITOR_MODE_OBJECT = 1,
};

/****************************************************************************
	Main class for node editor
	***************************************************************************/
class QtNodeEditor : public QWidget {
	Q_OBJECT

	qreal m_zoom;
	bool m_context_menu_enabled;
	bool m_menu_zoom_enabled;
	bool m_menu_selection_to_compound_enabled;
	bool m_menu_collapse_expand_enabled;
	bool m_menu_expand_compounds_enabled;
	bool m_menu_fisheye_view_enabled;
	bool m_fisheye_view_enabled;
	QMenu *m_context_menu;
	QMenu *m_zoom_sub_menu;
	QMenu *m_fisheye_view_sub_menu;
	QGraphicsView *m_view;
	QtNodeGraphicsScene *m_current_scene;
	QtNodeGraphicsScene *m_scene_scene;
	QVector<QtNode*> m_currently_selected_nodes;
	QVector<QtConnection*> m_currently_selected_connections;
	QtNode *m_last_removed_node;
	QGraphicsRectItem *m_rubber_band;
	bool m_rubberband_selection;
	QPointF m_last_mouse_position;
	QtCompoundNode *m_compound_node_dropped;
	QString m_header_title_icon;
	QString m_action_1_icon;
	QString m_action_2_icon;
	qreal m_fisheye_max_zoom;
	unsigned int m_fisheye_steps;
	int m_editor_mode = EDITOR_MODE_SCENE;

public:
	explicit QtNodeEditor(QWidget *parent = nullptr);
	virtual ~QtNodeEditor();

	bool eventFilter(QObject *object, QEvent *event) override;

	/* If true, a context menu is displayed when the right mousebutton is
	 * selected. */
	void setContextMenuEnabled(bool enabled);
	bool isContextMenuEnabled();

	/* If true, the context menu is extended with an option to zoom in/out. */
	void setMenuZoomEnabled(bool enabled);
	bool isMenuZoomEnabled();

	/* If true, the context menu is extended with an option to crop the selected
	 * nodes to a compound. */
	void setMenuSelectionToCompoundEnabled(bool enabled);
	bool isMenuSelectionToCompoundEnabled();

	/* If true, the context menu is extended with an option to exand/collapse
	 * all nodes. */
	void setMenuCollapseExpandEnabled(bool enabled);
	bool isMenuCollapseExpandEnabled();

	/* If true, the context menu is extended with an option to expand all
	 * compounds. This deletes all compound and releases the enclosed inner
	 * nodes. This action is done recursively. */
	void setMenuExpandCompoundsEnabled(bool enabled);
	bool isMenuExpandCompoundsEnabled();

	/* If true, the context menu is extended with an option to view in fisheye
	 * view. */
	void setMenuFisheyeViewEnabled(bool enabled);
	bool isMenuFisheyeViewEnabled();

	/* Set the filename for the compound (created by the context menu); if
	 * applicable. */
	void setCompoundHeaderTitleIcon(const QString &fileNameIcon);

	/* Set the action 1 icon for the compound (created by the context menu);
	 * if applicable. */
	void setCompoundAction1Icon(const QString &fileNameIcon);

	/* Set the action 2 icon for the compound (created by the context menu);
	 * if applicable. */
	void setCompoundAction2Icon(const QString &fileNameIcon);

	/* Add a node to the scene (editor widget). */
	void addNode(QtNode *node);

	/* Add a node to the scene (editor widget). */
	QVector<QtNode *> getNodes() const;

	/* Remove a node from the scene and destroy it.
	 * If cascade = true, any compound node is expanded (the compound itself is
	 * deleted) and the nodes inside become visible. */
	void removeNode(QtNode *node, bool cascade = false);

	/* Removes (and destroys) all selected nodes and connections from the scene
	 * (editor widget). */
	void removeAllSelelected();

	/* Center all nodes */
	void center();

	/* Remove a connection from the scene (editor widget) and destroy it. */
	void removeConnection(QtConnection *connection);

	/* Remove AND delete all items from the node editor scene. */
	void clear();

	/* If there is a selected node, return the last one that was selected. */
	QtNode *getLastSelectedNode() const;

	/* Return a vector with all the currently selected nodes. */
	const QVector<QtNode *> &getSelectedNodes() const;

	/* Return a vector with all the currently selected connections. */
	const QVector<QtConnection *> &getSelectedConnections() const;

	/* Return a pointer to the last removed node.
	 * Note, that the node is already deleted at this point, so this pointer
	 * does not point to a valid piece of memory. This function is only used to
	 * inform the caller about the deleted note. Prefered is to use a 'signal'
	 * (nodeToBeRemoved). */
	QtNode *getRemovedNode();

	/* Moves a node to front; before all other nodes. */
	void toFront(QtNode *node);

	/* Moves all other nodes in front of the given node. */
	void toBack (QtNode *node);

	/* Scale all nodes with the zoom factor. */
	void setZoom(qreal zoom);

	/* Scale all nodes with the zoom factor. */
	void setFisheyeView(bool enabled, qreal maxZoom = 1.0f, unsigned int steps = 5);

public Q_SLOTS:
	/* Activated when a contextmenu item is selected */
	void contextMenuItemSelected(QAction *action);

	/* Activated when a connection is set between two nodes. */
	void connectionEstablished(QtNode*, QtPort*, QtConnection*);

Q_SIGNALS:
	/* Emitted when a node is added to the node editor */
	void nodeAdded(QtNode *);

	/* Emitted when a node is going to be removed from the node editor */
	void nodeToBeRemoved(QtNode *);
	void objectNodeRemoved(ObjectNodeItem *);

	/* Emitted when a node is selected */
	void nodeSelected(QtNode *);
	void objectNodeSelected(ObjectNodeItem *);

	/* Emitted when one or more nodes are to be added to a compound; get these
	 * nodes by means of QtNodeEditor::getSelectedNodes. This signal is
	 * typically emitted when the contextmenu item is selectedwith function
	 * "Create compound from selected items". */
	void selectedNodesToBeAddedToCompound();

	/* Activated when a connection is set between two nodes. */
	void nodesConnected(QtNode *, const QString &, QtNode *, const QString &);

	/* Activated when a connection is removed between two nodes. */
	void connectionRemoved(QtNode *, const QString &, QtNode *, const QString &);

protected:
	QGraphicsItem *itemAtExceptActiveConnection(const QPointF &pos);
	QtCompoundNode *nodeOverCompound(QtNode *node);
	bool mouseClickHandler(QGraphicsSceneMouseEvent *mouseEvent);
	bool mouseDoubleClickHandler(QGraphicsSceneMouseEvent *mouseEvent);
	bool mouseMoveHandler(QGraphicsSceneMouseEvent *mouseEvent);
	bool mouseReleaseHandler(QGraphicsSceneMouseEvent *mouseEvent);
	void keyPressEvent(QKeyEvent *event) override;
	void rubberbandSelection(QGraphicsSceneMouseEvent *mouseEvent);
	void fisheyeZoom(QGraphicsSceneMouseEvent *mouseEvent);
	void deselectAll();
	void deleteAllActiveConnections();
	bool isActiveConnection();
	void deselectConnections();
	void deselectNodes();
	void selectNode(QtNode *node, QGraphicsSceneMouseEvent *mouseEvent);
	void selectConnection(QtConnection *connection);
	QtNode *getNodeWithActiveConnection();
	bool ctrlPressed();
	bool isNodeAlreadySelected(QtNode *node);
	bool isConnectionAlreadySelected(QtConnection *connection);
	void showContextMenu(const QPoint &pos);
	QAction *getActionFromContextMenu(const QString &actionText);
	void setZoom_For_Action(qreal zoom, QAction *action);
	void resetZoomSubmenu();
	void setFisheyeForAction(QAction *action, bool enabled, qreal maxZoom = 1.0f, unsigned int steps = 5);
	void resetFisheySubmenu();
	bool isNode(QGraphicsItem *item) const;
	bool isCompoundNode(QGraphicsItem *item);
	bool isConnection(QGraphicsItem *item);
	void wheelEvent(QWheelEvent *event) override;
};

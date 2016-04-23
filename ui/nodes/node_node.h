/****************************************************************************
**
* *Copyright(C) 2014
**
* *This file is generated by the Magus toolkit
**
* *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* *LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* *A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* *OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* *SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT
* *LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* *DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* *THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#pragma once

#include <QPen>

#include "node_constants.h"
#include "node_port.h"

class Node;

static constexpr auto NODE_ACTION_BASE = 1;
static constexpr auto NODE_ACTION_TARGET = 2;

class QtPort;
class QtConnection;
class QtNodeEditor;

/****************************************************************************
	QtNode represents a Node-class which is visualised in a QGraphicsScene.
	***************************************************************************/
class QtNode : public QObject, public QGraphicsPathItem {
	Q_OBJECT

	Node *m_data;

	bool m_auto_size;
	qreal m_icon_size;
	qreal m_width;
	qreal m_normalized_width;
	qreal m_normalized_body_height;
	qreal m_header_height;
	qreal m_body_height;
	QGraphicsPathItem *m_body;
	QGraphicsPixmapItem *m_header_title_icon;
	QGraphicsPixmapItem *m_action_1_icon;
	QGraphicsPixmapItem *m_action_2_icon;
	QBrush m_header_brush;
	QPen m_pen;
	QtNodeEditor *m_editor;
	QGraphicsScene *m_scene;
	QString m_title;
	QGraphicsTextItem *m_title_label;
	QFont m_font_header;
	Alignment m_title_alignment;

	QVector<QtPort *> m_port_list;
	QVector<QtPort *> m_input_ports;
	QVector<QtPort *> m_output_ports;

	QColor m_port_name_color;
	QGraphicsItem *m_original_parent;
	QPixmap m_image;
	QGraphicsPixmapItem *m_pixmap_item;
	bool m_image_set;

public:
	QtConnection *m_active_connection;

	QtNode(const QString &title, QGraphicsItem *parent = nullptr);
	virtual ~QtNode() = default;

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

	void setNode(Node *node);

	Node *getNode() const;

	/* Set the editor; this is called(by the editor) as soon as a node is added
	 * to the scene. Do not use this(only for internal use) */
	void setEditor(QtNodeEditor *editor);

	/* Set the scene; this is called(by the editor) as soon as a node is added
	 * to the scene. Do not use this (only for internal use) */
	void setScene(QGraphicsScene *scene);

	/* Called from the QNodeEditor; used for adding a node to a compound */
	/* Overridden */
	void setParentItem(QGraphicsItem *parent);

	/* Called from the QNodeEditor; used for removing a node from a compound */
	void restoreOriginalParentItem();

	/* To be called just before the node is removed from the scene */
	virtual void prepareDelete();

	/* Set the node - including its connections - visible / invisible */
	void setVisible(bool visible);

	/* Determines whether a node is automatically sized when ports are added etc. */
	void setAutoSize(bool autoSize);
	bool isAutoSize() const;

	/* Return the title of the node */
	const QString &getTitle() const;

	/* Although the width of the node is automatically adjusted to its content,
	 * it is possible to change (override) the width manually */
	void setWidth(qreal width);

	/* Set the height of the header */
	void setHeaderHeight(qreal headerHeight);

	/* Although the body height of the node is automatically adjusted to its
	 * content, it is possible to change (override) the body height manually. */
	void setBodyHeight(qreal bodyHeight);

	/* Get the dimensions of the node (width and total height) */
	qreal getWidth() const;
	qreal getHeigth() const;

	/* Set the color of the header; this results in a gradient with grey */
	void setHeaderColor(const QColor &color);

	/* Set the color of the title in the header */
	void setTitleColor(const QColor &color);

	/* Set the color of the name of all ports */
	void setPortNameColor(const QColor &color);

	/* Align the text of the header (default is center) */
	void alignTitle(Alignment alignment);

	/* Set the size of the icon in the header */
	void setIconSize(qreal size);

	/* Set the icon, associated with the title in the nodes' header */
	void setHeaderTitleIcon(const QString &fileNameIcon);

	/* Set the icon, associated with action 1 */
	void setAction1Icon(const QString &fileNameIcon);

	/* Set the icon, associated with action 2 */
	void setAction2Icon(const QString &fileNameIcon);

	/* Create a port
	 * portId:          Identification (handle) of the port.
	 * portName:        The name of the port. This is also the text displayed.
	 * type:            Define the type of port. By means of the portType it is also possible to define whether it is
	 *                  possible to connect this port to another port (of another or the same type). By default, 2 prefab
	 *                  portTypes exist, a QtInputPortType and a QtOutputPortType. It is also possible to create your own
	 *                  port type.
	 * portColour:      The color of a port.
	 * portShape:       A port can be shaped circular or as a square.
	 * alignement:      A port can be positioned left, right or in the middle of a node.
	 * connectionColor: If connecting is started from this port, it gets the color defined by 'connectionColor'.
	 *                  The created port is in that case the 'base port' of the connection.
	 */
	QtPort *createPort(const QString &portName,
	                   int type,
	                   QColor portColour,
	                   QtPortShape portShape,
	                   Alignment alignement,
	                   QColor connectionColor = Qt::black);

	/* Add an image to the node; the image is adjusted to the size of the node
	 * and position at the top (under the header) */
	void setImage(const QString &fileNameImage);
	void setImage(const QPixmap &pixMap);

	/* Add an image to the node; the node is adjusted to the size of the image */
	void createImage(const QString &fileNameImage, QSize size);

	/* Return the port, based on the name */
	QtPort *getPort(const QString &portName);

	/* Return the port, based on the name.
	 * In case there are multiple ports with the same name, the first occurence
	 * is returned; use the occurence argument in case of more than one port
	 * with the same name; occurence starts with 1 */
	QtPort *getPort(const QString &portName, unsigned int occurence);

	/* Return all ports of a node */
	QVector<QtPort *> getPorts();

	/* If there are multiple ports with the same name, they can also be returned
	 * as a vector */
	QVector<QtPort *> getPorts(const QString &portName);

	/* Returns true, if a given port is part of 'this' node */
	bool isPortOfThisNode(QtPort *port);
	bool isPortOfThisNode(const QString &portName);

	/* Returns true, if a given connection is connected to a port of 'this' node */
	bool isConnectionConnectedToThisNode(QtConnection *connection);

	/* Return the node that is connected to 'this' node by means of a given port
	 * (identified by its name or a pointer to the port).
	 * Returns 0 if there is no connection */
	QtNode *getNodeConnectedToPort(QtPort *port);

	/* Return the node that is connected to 'this' node by means of a given portName */
	/* Returns 0 if there is no connection */
	QtNode *getNodeConnectedToPort(const QString &portName);

	/* Return the node that is connected to 'this' node by means of a given
	 * portName. In case there are multiple ports with the same name, the first
	 * occurence is returned; use the occurence argument in case of more than
	 * one port with the same name; occurence starts with 1.
	 * Returns nullptr if there is no connection */
	QtNode *getNodeConnectedToPort(const QString &portName, unsigned int occurence);

	/* If there are multiple ports with the same name, the nodes can also be
	 * returned as a vector */
	QVector<QtNode*> getNodesConnectedToPorts(const QString &portName);

	/* Return the port that is connected to the port of 'this' node.
	 * Returns nullptr if there is no connection */
	QtPort *getPortConnectedToPort(QtPort *port);

	/* Return the port that is connected to the port of 'this' node.
	 * Returns nullptr if there is no connection */
	QtPort *getPortConnectedToPort(const QString &portName);

	/* Return the port that is connected to the port of 'this' node.
	 * In case there are multiple ports with the same name, the first occurence
	 * is returned; use the occurence argument in case of more than one port
	 * with the same name; occurence starts with 1.
	 * Returns nullptr if there is no connection */
	QtPort *getPortConnectedToPort(const QString &portName, unsigned int occurence);

	/* If there are multiple ports with the same name, the ports can also be returned as a vector */
	QVector<QtPort *> getPortsConnectedToPorts(const QString &portName);

	/* Create an active connection that must be connected to another node (using
	 * the mouse).
	 * port = The base port to which the connection is attached; this is also
	 *        the start position.
	 * pos = End position */
	void createActiveConnection(QtPort *port, QPointF pos);

	/* Delete the connection that is currently active */
	void deleteActiveConnection();

	/* Delete all connections, connected to this node */
	void deleteAllConnections();

	/* Select or deselect all connections; this only applies to finalized
	 * (established) connections */
	void selectConnections(bool selected);

	/* Called from the QNodeEditor when either a left mouse click on the action1
	 * or action2 button has been done. This handler function performs default
	 * behaviour, but it is possible to overload these functions.
	 * mouseLeftClickAction1ButtonHandler: Toggle collapse and expand the node
	 * mouseLeftClickAction2ButtonHandler: Close (delete) the node */
	virtual void mouseLeftClickAction1ButtonHandler(QGraphicsSceneMouseEvent *mouseEvent, QGraphicsItem *item);
	virtual void mouseLeftClickAction2ButtonHandler(QGraphicsSceneMouseEvent *mouseEvent, QGraphicsItem *item);

	/* Called from the QNodeEditor when either a left mouse click on the QtNode
	 * has taken place, or on one of its child QGraphicsItems. The
	 * mouseLeftClickHandler function performs default behaviour, but it is
	 * possible to overload this function. */
	virtual bool mouseLeftClickHandler(QGraphicsSceneMouseEvent *mouseEvent,
	                                   QGraphicsItem *item,
	                                   unsigned int action = 0,
	                                   QtConnection *activeConnection = nullptr);

	/* Called from the QNodeEditor when either a right mouse click on the QtNode
	 * has taken place, or on one of its child QGraphicsItems. */
	virtual bool mouseRightClickHandler(QGraphicsSceneMouseEvent *mouseEvent,
	                                    QGraphicsItem *item,
	                                    unsigned int action = 0,
	                                    QtConnection *activeConnection = nullptr);

	/* Called from the QNodeEditor when either a double click on the QtNode has
	 * taken place, or on one of its child QGraphicsItems. The
	 * mouseDoubleClickHandler function performs default behaviour, but it is
	 * possible to overload this function. */
	virtual bool mouseDoubleClickHandler(QGraphicsSceneMouseEvent *mouseEvent, QGraphicsItem *item);

	/* Called from the QNodeEditor. */
	virtual bool mouseMoveHandler(QGraphicsSceneMouseEvent *mouseEvent, QGraphicsItem *item);

	/* Collapse the node (and hide the ports) */
	void collapse();

	/* Make all ports visible and restore the endpoint of the connection */
	void expand();

	bool hasInputs() const;
	bool hasOutputs() const;

	QtPort *input(int index) const;
	QtPort *output(int index) const;

Q_SIGNALS:
	/* Emitted when the action 1 icon is clicked */
	void action1Clicked(QtNode*);

	/* Emitted when the action 2 icon is clicked */
	void action2Clicked(QtNode*);

protected:
	void setTitlePosition();
	void adjustWidthForTitle();
	void setPortAlignedPos(QtPort *port, qreal height);
	void redraw();

	QtPort *getCheckedPortConnectedToPort(QtPort *port);
};

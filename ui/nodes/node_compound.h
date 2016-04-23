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

#include "node_node.h"

class Object;
class QtNodeGraphicsScene;

class ObjectNodeItem : public QtNode {
	Q_OBJECT

	QVector<QtNode *> m_node_list;
	QtNodeGraphicsScene *m_node_scene;
	Object *m_object;

public:
	ObjectNodeItem(Object *object, const QString &title, QGraphicsItem *parent = nullptr);

	virtual ~ObjectNodeItem();

	/* Overridden from parent */
	virtual void prepareDelete() override;

	/* Add a node to the compound; it becomes invisible.
	 * Nodes that are connected to the added node, remain connected, but via the
	 * compound. The connected port is displayed in the compound. */
	void addNode(QtNode *node);

	/* Remove a node from the compound; it becomes visible again (the node is
	 * not deleted). A removed node that still has a connection with a node in
	 * the compound remains connected (but with the compound). The port of the
	 * connected node is part of and displayed in the compound. */
	void removeNode(QtNode *node);

	/* Returns true in case the given node is included in this compound. */
	bool isNodeOfThisCompound(QtNode *node) const;

	/* Returns the vector with nodes added to this compound.
	 * Note, that the connections of the returned nodes are not correct as long
	 * as the nodes are part of the compound. The connections are rewired, so
	 * the internal relations of the nodes are changed. Only if the compound is
	 * removed (deleted), its added nodes are released and their original
	 * relationships are restored. */
	const QVector<QtNode *> &getNodes() const;

	QtNodeGraphicsScene *nodeScene() const;

	/* Default behaviour is that the added nodes are released and the compound
	 * deleted. */
//	virtual void mouseLeftClickAction2ButtonHandler(QGraphicsSceneMouseEvent *mouseEvent, QGraphicsItem *item);

	Object *object() const;

Q_SIGNALS:
	/* Emitted when the a node is added to the compound. */
	void nodeAdded(QtNode *);

	/* Emitted when the a node is removed (not deleted) from the compound. */
	void nodeRemoved(QtNode *);
};

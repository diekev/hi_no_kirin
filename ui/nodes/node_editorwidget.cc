/****************************************************************************
**
* *Copyright (C) 2014
**
* *This file is generated by the Magus toolkit
**
* *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* *LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* *A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* *OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* *SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* *LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* *DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* *THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#include "node_editorwidget.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QToolTip>
#include <QVBoxLayout>

#include <iostream>
#include <kamikaze/context.h>
#include <kamikaze/operateur.h>
#include <kangao/kangao.h>
#include <sstream>

#include "node_compound.h"
#include "node_connection.h"
#include "node_node.h"
#include "node_port.h"
#include "node_scene.h"

#include "object.h"
#include "graphs/object_graph.h"
#include "operateurs/operateurs_standards.h"

/* ************************************************************************** */

QtNodeEditor::QtNodeEditor(
		kangao::RepondantBouton *repondant,
		kangao::GestionnaireInterface *gestionnaire,
		QWidget *parent)
    : WidgetBase(parent)
    , m_view(new NodeView(this))
    , m_graphics_scene(new QtNodeGraphicsScene())
	, m_gestionnaire(gestionnaire)
{
	m_main_layout->addWidget(m_view);

	m_graphics_scene->installEventFilter(this);

	/* Hide graphics view's frame. */
	m_view->setStyleSheet("border: 0px");
	m_view->setScene(m_graphics_scene);
	m_view->setRenderHint(QPainter::Antialiasing, true);
	m_view->setInteractive(true);
	m_view->setMouseTracking(true);
	m_view->setBackgroundBrush(QBrush(QColor(127, 127, 127)));

	m_rubber_band = nullptr;
	m_hover_connection = nullptr;
	m_active_connection = nullptr;
	m_rubberband_selection = false;
	m_context_menu_enabled = true;

	kangao::DonneesInterface donnees;
	donnees.manipulable = nullptr;
	donnees.conteneur = nullptr;
	donnees.repondant_bouton = repondant;

	const auto texte_entree = kangao::contenu_fichier("interface/menu_editeur_noeud.kangao");
	m_context_menu = m_gestionnaire->compile_menu(donnees, texte_entree.c_str());

	setContextMenuPolicy(Qt::CustomContextMenu);
}

QtNodeEditor::~QtNodeEditor()
{
	delete m_context_menu;
	delete m_graphics_scene;
}

void QtNodeEditor::setContextMenuEnabled(bool enabled)
{
	m_context_menu_enabled = enabled;
}

bool QtNodeEditor::isContextMenuEnabled()
{
	return m_context_menu_enabled;
}


QGraphicsItem *QtNodeEditor::itemAtExceptActiveConnection(const QPointF &pos)
{
	const auto &items = m_graphics_scene->items(QRectF(pos - QPointF(1, 1), QSize(3, 3)));
	const auto is_active = (m_active_connection != nullptr);

	/* If there is an active connection, it is not returned as a selected item.
	 * Finalized (established) connections are returned. */
	for (const auto &item : items) {
		if (!item->isVisible()) {
			continue;
		}

		if (!is_connection(item)) {
			return item;
		}

		if (!is_active) {
			return item;
		}
	}

	return nullptr;
}

QtConnection *QtNodeEditor::nodeOverConnection(QtNode *node)
{
	if (!node->hasInputs() || !node->hasOutputs()) {
		return nullptr;
	}

	/* already connected */
	if (node->input(0)->isConnected() || node->output(0)->isConnected()) {
		return nullptr;
	}

	auto check_hover = [&](QtNode *nitem, QtConnection *citem)
	{
		const auto &bport_pos_x = citem->getBasePort()->scenePos().x();
		const auto &bport_pos_y = citem->getBasePort()->scenePos().y();
		const auto &tport_pos_x = citem->getTargetPort()->scenePos().x();
		const auto &tport_pos_y = citem->getTargetPort()->scenePos().y();

		const auto &min_x = std::min(bport_pos_x, tport_pos_x);
		const auto &min_y = std::min(bport_pos_y, tport_pos_y);
		const auto &max_x = std::max(bport_pos_x, tport_pos_x);
		const auto &max_y = std::max(bport_pos_y, tport_pos_y);

		const auto &pos_x = nitem->pos().x();
		const auto &pos_y = nitem->pos().y();

		return (min_x < pos_x && max_x > pos_x) && (min_y < pos_y && max_y > pos_y);
	};

	if (m_hover_connection && check_hover(node, m_hover_connection)) {
		return m_hover_connection;
	}

	QtConnection *connection;

	for (const auto &item : m_graphics_scene->items()) {
		if (!is_connection(item)) {
			continue;
		}

		connection = static_cast<QtConnection *>(item);

		if (connection->isSelected()) {
			continue;
		}

		if (node->isConnectionConnectedToThisNode(connection)) {
			continue;
		}

		if (check_hover(node, connection)) {
			return connection;
		}
	}

	if (m_hover_connection) {
		m_hover_connection->setSelected(false);
	}

	return nullptr;
}

bool QtNodeEditor::eventFilter(QObject *object, QEvent *event)
{
	auto mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);

	switch (static_cast<int>(event->type())) {
		case QEvent::GraphicsSceneMousePress:
		{
			this->set_active();
			mouseClickHandler(mouseEvent);
			break;
		}
		case QEvent::GraphicsSceneMouseDoubleClick:
		{
			this->set_active();

			if (mouseDoubleClickHandler(mouseEvent)) {
				return true;
			}

			break;
		}
		case QEvent::GraphicsSceneMouseMove:
		{
			mouseMoveHandler(mouseEvent);
			break;
		}
		case QEvent::GraphicsSceneMouseRelease:
		{
			mouseReleaseHandler(mouseEvent);
			break;
		}
	}

	return QObject::eventFilter(object, event);
}

bool QtNodeEditor::mouseClickHandler(QGraphicsSceneMouseEvent *mouseEvent)
{
	switch (static_cast<int>(mouseEvent->button())) {
		case Qt::LeftButton:
		{
			m_mouse_down = true;

			const auto &item = itemAtExceptActiveConnection(mouseEvent->scenePos());

			if (!item) {
				/* Left-click on the canvas, but no item clicked, so deselect nodes and connections */
				deselectAll();
				m_rubberband_selection = true;
				m_last_mouse_position.setX(mouseEvent->lastScenePos().x());
				m_last_mouse_position.setY(mouseEvent->lastScenePos().y());
				return true;
			}

			m_rubberband_selection = false;

			/* Did not click on a node. */
			if (!item->data(NODE_KEY_GRAPHIC_ITEM_TYPE).isValid()) {
				/* TODO: shouldn't we return false? */
				return true;
			}

			/* Delegate to the node; either the node itself is clicked, one of
			 * its children or a connection.
			 */

			const auto type = item->data(NODE_KEY_GRAPHIC_ITEM_TYPE).toInt();

			switch (type) {
				case NODE_VALUE_TYPE_CONNECTION:
				{
					deselectNodes();
					selectConnection(static_cast<QtConnection *>(item));
					break;
				}
				case NODE_VALUE_TYPE_NODE:
				{
					selectNode(static_cast<QtNode *>(item), mouseEvent);
					break;
				}
				case NODE_VALUE_TYPE_HEADER_ICON:
				case NODE_VALUE_TYPE_HEADER_TITLE:
				case NODE_VALUE_TYPE_NODE_BODY:
				{
					if (is_node(item->parentItem()) || is_object_node(item->parentItem())) {
						selectNode(static_cast<QtNode *>(item->parentItem()), mouseEvent);
					}

					break;
				}
				case NODE_VALUE_TYPE_PORT:
				{
					deselectNodes();
					deselectConnections();

					auto node = static_cast<QtNode *>(item->parentItem());

					/* Either make a connection to another port, or create a new
					 * connection */
					auto baseNode = nodeWithActiveConnection();

					if (m_active_connection == nullptr) {
						/* There is no active connection, so start one */
						node->mouseLeftClickHandler(mouseEvent, item, NODE_ACTION_BASE);
						m_active_connection = node->m_active_connection;
					}
					else if (baseNode != node) {
						/* There is an active connection and the selected
						 * port is not part of the baseNode, so try to
						 * establish a connection with the other node */
						if (node->mouseLeftClickHandler(mouseEvent, item,
						                                NODE_ACTION_TARGET,
						                                baseNode->m_active_connection))
						{
							m_active_connection = nullptr;
						}
					}

					break;
				}
				default:
				{
					deselectNodes();
					deselectConnections();

					auto node = static_cast<QtNode *>(item->parentItem());
					node->mouseLeftClickHandler(mouseEvent, item);

					break;
				}
			}

			break;
		}
		case Qt::RightButton:
		{
			if (m_context_menu_enabled) {
				QPoint pos;
				pos.setX(mouseEvent->lastScreenPos().x());
				pos.setY(mouseEvent->lastScreenPos().y());
				showContextMenu(pos);
			}
			else {
				deselectAll();
			}

			return true;
		}
		case Qt::MiddleButton:
		{
			const auto &item = itemAtExceptActiveConnection(mouseEvent->scenePos());

			if (!item) {
				return true;
			}

			if (!item->data(NODE_KEY_GRAPHIC_ITEM_TYPE).isValid()) {
				/* TODO: shouldn't we return false? */
				return true;
			}

			const auto type = item->data(NODE_KEY_GRAPHIC_ITEM_TYPE).toInt();

			Noeud *noeud = nullptr;

			switch (type) {
				case NODE_VALUE_TYPE_NODE_BODY:
				case NODE_VALUE_TYPE_HEADER_TITLE:
				case NODE_VALUE_TYPE_HEADER_ICON:
				{
					noeud = static_cast<QtNode *>(item->parentItem())->pointeur_noeud();
					break;
				}
				case NODE_VALUE_TYPE_NODE:
				{
					noeud = static_cast<QtNode *>(item)->pointeur_noeud();
					break;
				}
				case NODE_VALUE_TYPE_CONNECTION:
				case NODE_VALUE_TYPE_PORT:
				default:
				{
					break;
				}
			}

			if (noeud != nullptr) {
				auto operateur = noeud->operateur();

				std::stringstream ss;
				ss << "<p>Opérateur : " << noeud->nom() << "</p>";
				ss << "<hr/>";
				ss << "<p>Temps d'exécution :";
				ss << "<p>- dernière : " << operateur->temps_execution() << " secondes.</p>";
				ss << "<p>- minimum : " << operateur->min_temps_execution() << " secondes.</p>";
				ss << "<p>- agrégé : " << operateur->temps_agrege() << " secondes.</p>";
				ss << "<p>- minimum agrégé : " << operateur->min_temps_agrege() << " secondes.</p>";
				ss << "<hr/>";
				ss << "<p>Nombre d'exécution : " << operateur->nombre_executions() << "</p>";
				ss << "<hr/>";

				QToolTip::showText(mouseEvent->screenPos(), ss.str().c_str());
			}

			return true;
		}
	}

	return true;
}

bool QtNodeEditor::mouseDoubleClickHandler(QGraphicsSceneMouseEvent *mouseEvent)
{
	switch (static_cast<int>(mouseEvent->button())) {
		case Qt::LeftButton:
		{
			const auto &item = itemAtExceptActiveConnection(mouseEvent->scenePos());

			if (!item) {
				/* Double-click on the canvas. */
				return true;
			}

			/* Did not click on a node. */
			if (!item->data(NODE_KEY_GRAPHIC_ITEM_TYPE).isValid()) {
				/* TODO: shouldn't we return false? */
				return true;
			}

			/* Delegate to the node; either the node itself is clicked, one of
			 * its children or a connection.
			 */

			const auto type = item->data(NODE_KEY_GRAPHIC_ITEM_TYPE).toInt();

			switch (type) {
				case NODE_VALUE_TYPE_NODE:
				{
					if (is_object_node(item)) {
						enterObjectNode(nullptr);
					}

					break;
				}
				case NODE_VALUE_TYPE_NODE_BODY:
				{
					if (is_object_node(item->parentItem())) {
						enterObjectNode(nullptr);
					}

					break;
				}
				case NODE_VALUE_TYPE_HEADER_TITLE:
				{
					/* This is handled in TextItem::mouseDoubleClickEvent. */
					return false;
				}
				default:
				{
					return false;
				}
			}
		}
	}

	return true;
}

void QtNodeEditor::enterObjectNode(QAction */*action*/)
{
	/* À FAIRE */
	m_context->eval_ctx->edit_mode = true;
	m_context->scene->notify_listeners(event_type::node | event_type::selected);
}

bool QtNodeEditor::mouseMoveHandler(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (!m_mouse_down) {
		return true;
	}

	/* If there was a rubberband selection started, update its rectangle */
	if (m_rubberband_selection && (mouseEvent->buttons() & Qt::LeftButton)) {
		rubberbandSelection(mouseEvent);
	}

	if (m_selected_nodes.size() == 1) {
		auto node = static_cast<QtNode *>(getLastSelectedNode());
		m_hover_connection = nodeOverConnection(node);

		if (m_hover_connection) {
			/* TODO: this is to highlight the conection, consider having a
			 * separate flag/colour for this. */
			m_hover_connection->setSelected(true);
			return true;
		}
	}

	if (m_active_connection) {
		m_active_connection->updatePath(mouseEvent->scenePos());
	}

	m_hover_connection = nullptr;
	setCursor(Qt::ArrowCursor);

	return true;
}

bool QtNodeEditor::mouseReleaseHandler(QGraphicsSceneMouseEvent *mouseEvent)
{
	m_mouse_down = false;

	/* Determine whether a node has been dropped on a connection. */
	if (m_hover_connection) {
		splitConnectionWithNode(getLastSelectedNode());
		m_hover_connection = nullptr;
		return true;
	}

	/* Handle the rubberband selection, if applicable */
	if (!m_rubberband_selection) {
		return false;
	}

	if (mouseEvent->button() & Qt::LeftButton) {
		if (m_rubber_band) {
			auto minX = qMin(m_last_mouse_position.x(), mouseEvent->lastScenePos().x());
			auto maxX = qMax(m_last_mouse_position.x(), mouseEvent->lastScenePos().x());
			auto minY = qMin(m_last_mouse_position.y(), mouseEvent->lastScenePos().y());
			auto maxY = qMax(m_last_mouse_position.y(), mouseEvent->lastScenePos().y());

			/* Select the items */
			const auto &items = m_graphics_scene->items();
			for (const auto &item : items) {
				if (!item->isVisible()) {
					continue;
				}

				if (is_connection(item)) {
					auto connection = static_cast<QtConnection *>(item);
					auto prise_base = connection->getBasePort();
					auto prise_cible = connection->getTargetPort();

					if (prise_base && prise_cible) {
						const auto &pos_prise_base = prise_base->scenePos();
						const auto &pos_prise_cible = prise_cible->scenePos();

						auto item_Min_X = qMin(pos_prise_base.x(), pos_prise_cible.x());
						auto item_Max_X = qMax(pos_prise_base.x(), pos_prise_cible.x());
						auto item_Min_Y = qMin(pos_prise_base.y(), pos_prise_cible.y());
						auto item_Max_Y = qMax(pos_prise_base.y(), pos_prise_cible.y());

						if (item_Min_X > minX && item_Max_X < maxX &&
						    item_Min_Y > minY && item_Max_Y < maxY)
						{
							selectConnection(connection);
						}
					}
				}
				else if (is_node(item)) {
					auto noeud = static_cast<QtNode *>(item);
					const auto &pos_neoud = noeud->scenePos();
					const auto &cadre_noeud = noeud->sceneBoundingRect();

					auto item_Min_X = pos_neoud.x() - 0.5 * cadre_noeud.width() + 10;
					auto item_Min_Y = pos_neoud.y() - 0.5 * cadre_noeud.height() + 10;
					auto item_Max_X = item_Min_X + cadre_noeud.width() + 10;
					auto item_Max_Y = item_Min_Y + cadre_noeud.height()  + 10;

					if (item_Min_X > minX && item_Max_X < maxX &&
					    item_Min_Y > minY && item_Max_Y < maxY)
					{
						selectNode(noeud, nullptr);
					}
				}
			}

			m_rubber_band->hide();
		}

		m_rubberband_selection = false;
	}

	return true;
}

void QtNodeEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Delete) {
		removeAllSelelected();
	}
	else if (event->key() == Qt::Key_A) {
		if (m_context->eval_ctx->edit_mode == true) {
			m_add_node_menu->popup(QCursor::pos());
		}
	}
}

void QtNodeEditor::rubberbandSelection(QGraphicsSceneMouseEvent *mouseEvent)
{
	/* Mouse is pressed and moves => draw rubberband */
	const auto x = mouseEvent->lastScenePos().x();
	const auto y = mouseEvent->lastScenePos().y();

	if (!m_rubber_band) {
		m_rubber_band = new QGraphicsRectItem(m_last_mouse_position.x(), m_last_mouse_position.y(), 0.0f, 0.0f);
		m_rubber_band->setPen(QPen(Qt::darkBlue));
		QColor c(Qt::darkBlue);
		c.setAlpha(64);
		m_rubber_band->setBrush(c);
		m_graphics_scene->addItem(m_rubber_band);
	}

	m_rubber_band->show();

	const auto minX = std::min(static_cast<qreal>(m_last_mouse_position.x()), x);
	const auto maxX = std::max(static_cast<qreal>(m_last_mouse_position.x()), x);
	const auto minY = std::min(static_cast<qreal>(m_last_mouse_position.y()), y);
	const auto maxY = std::max(static_cast<qreal>(m_last_mouse_position.y()), y);

	m_rubber_band->setRect(minX, minY, maxX - minX, maxY - minY);
}

void QtNodeEditor::selectNode(QtNode *node, QGraphicsSceneMouseEvent *mouseEvent)
{
	if (!ctrlPressed()) {
		deselectAll();
	}

	if (is_object_node(node)) {
		setActiveObject(static_cast<ObjectNodeItem *>(node));
	}
	else {
		auto object = static_cast<Object *>(m_context->scene->active_node());

		auto graph = object->graph();
		graph->ajoute_selection(node->pointeur_noeud());

		m_context->scene->notify_listeners(event_type::node | event_type::selected);
	}
}

void QtNodeEditor::selectConnection(QtConnection *connection)
{
	if (!ctrlPressed()) {
		deselectAll();
	}

	if (!isAlreadySelected(connection)) {
		auto object = static_cast<Object *>(m_context->scene->active_node());
		auto graph = object->graph();
		graph->ajoute_selection(connection->pointeur_lien());

		m_selected_connections.append(connection);
		connection->setSelected(true);
	}
}

void QtNodeEditor::deselectAll()
{
	setCursor(Qt::ArrowCursor);
	deleteAllActiveConnections();
	deselectConnections();
	deselectNodes();
}

void QtNodeEditor::deleteAllActiveConnections()
{
	const auto &items = m_graphics_scene->items();
	QtNode *node;

	for (const auto &item : items) {
		if (is_node(item) && item->isVisible()) {
			node = static_cast<QtNode *>(item);
			node->deleteActiveConnection();
		}
	}

	m_active_connection = nullptr;
}

void QtNodeEditor::deselectConnections()
{
	for (const auto &connection : m_selected_connections) {
		if (connection->isVisible()) {
			connection->setSelected(false);
		}
	}

	m_selected_connections.clear();
}

void QtNodeEditor::deselectNodes()
{
	if (m_context->eval_ctx->edit_mode) {
		auto object = static_cast<Object *>(m_context->scene->active_node());
		auto graph = object->graph();

		for (const auto &node : m_selected_nodes) {
			graph->enleve_selection(node->pointeur_noeud());
		}
	}
	else {
		/* TODO */
	}

	m_selected_nodes.clear();
}

QtNode *QtNodeEditor::nodeWithActiveConnection()
{
	if (!m_active_connection) {
		return nullptr;
	}

	return static_cast<QtNode *>(m_active_connection->getBasePort()->parentItem());
}

void QtNodeEditor::addNode(QtNode *node)
{
	node->setEditor(this);
	node->setScene(m_graphics_scene);

	m_graphics_scene->addItem(node);

	m_hover_connection = lastSelectedConnection();

	if (m_hover_connection != nullptr) {
		splitConnectionWithNode(node);
	}

	m_hover_connection = nullptr;
}

QVector<QtNode *> QtNodeEditor::getNodes() const
{
	const auto &items = m_graphics_scene->items();
	auto nodeList = QVector<QtNode *>{};

	QtNode *node;

	for (const auto &item : items) {
		if (is_node(item) && item->isVisible()) {
			node = static_cast<QtNode *>(item);
			nodeList.append(node);
		}
	}

	return nodeList;
}

void QtNodeEditor::removeNode(QtNode *node)
{
	if (!node) {
		return;
	}

	if (is_object_node(node)) {
		auto object = static_cast<ObjectNodeItem *>(node)->scene_node();
		m_context->scene->removeObject(object);
	}
	else {
		auto scene = m_context->scene;
		auto object = static_cast<Object *>(scene->active_node());
		auto graph = object->graph();
		auto pointeur_noeud = node->pointeur_noeud();
		const auto avait_liaison = pointeur_noeud->est_connecte();

		graph->enleve(pointeur_noeud);

		if (avait_liaison) {
			scene->evalObjectDag(*m_context, object);
		}

		m_context->scene->notify_listeners(event_type::node | event_type::removed);
	}
}

using node_port_pair = std::pair<QtNode *, QtPort *>;

std::pair<node_port_pair, node_port_pair> get_base_target_pairs(QtConnection *connection, bool remove)
{
	auto base_port = connection->getBasePort();
	auto target_port = connection->getTargetPort();

	if (remove) {
		base_port->deleteConnection(connection);
	}

	/* make sure the connection is set in the right order */
	if (target_port->isOutputPort() && !base_port->isOutputPort()) {
		std::swap(base_port, target_port);
	}

	const auto &base_node = static_cast<QtNode *>(base_port->parentItem());
	const auto &target_node = static_cast<QtNode *>(target_port->parentItem());

	return {
		{ base_node, base_port },
		{ target_node, target_port }
	};
}

void QtNodeEditor::removeConnection(QtConnection *connection)
{
	const auto &pairs = get_base_target_pairs(connection, true);
	const auto &base = pairs.first;
	const auto &target = pairs.second;

	connectionRemoved(base.first, base.second->getPortName(),
	                  target.first, target.second->getPortName(), true);
}

void QtNodeEditor::splitConnectionWithNode(QtNode *node)
{
	const auto &connection = m_hover_connection;

	const auto &pairs = get_base_target_pairs(connection, true);
	const auto &base = pairs.first;
	const auto &target = pairs.second;

	auto scene = m_context->scene;
	auto object = static_cast<Object *>(scene->active_node());
	auto graph = object->graph();

	auto noeud_de = base.first->pointeur_noeud();
	auto noeud_a = target.first->pointeur_noeud();

	/* remove connection */
	auto prise_sortie = noeud_de->sortie(base.second->getPortName().toStdString());
	auto prise_entree = noeud_a->entree(target.second->getPortName().toStdString());

	assert((prise_sortie != nullptr) && (prise_entree != nullptr));

	graph->deconnecte(prise_sortie, prise_entree);

	auto noeud_milieu = node->pointeur_noeud();

	/* connect from base port to first input port in node */
	graph->connecte(prise_sortie, noeud_milieu->entree(0));

	/* connect from first output port in node to target port */
	graph->connecte(noeud_milieu->sortie(0), prise_entree);

	/* notify */
	scene->evalObjectDag(*m_context, object);
	scene->notify_listeners(event_type::node | event_type::modified);
}

void QtNodeEditor::connectionEstablished(QtConnection *connection)
{
	const auto &pairs = get_base_target_pairs(connection, false);
	const auto &base = pairs.first;
	const auto &target = pairs.second;

	nodesConnected(base.first, base.second->getPortName(),
	               target.first, target.second->getPortName(), true);
}

void QtNodeEditor::removeAllSelelected()
{
	for (auto &connection : m_selected_connections) {
		removeConnection(connection);
	}

	m_selected_connections.clear();

	for (auto &node : m_selected_nodes) {
		removeNode(node);
	}

	m_selected_nodes.clear();
}

void QtNodeEditor::clear()
{
	m_graphics_scene->clear(); /* removes + deletes all items in the scene */
}

QtNode *QtNodeEditor::getLastSelectedNode() const
{
	if (m_selected_nodes.empty()) {
		return nullptr;
	}

	return m_selected_nodes.back();
}

const QVector<QtNode *> &QtNodeEditor::selectedNodes() const
{
	return m_selected_nodes;
}

QtConnection *QtNodeEditor::lastSelectedConnection() const
{
	if (m_selected_connections.empty()) {
		return nullptr;
	}

	return m_selected_connections.back();
}

const QVector<QtConnection *> &QtNodeEditor::selectedConnections() const
{
	return m_selected_connections;
}

void QtNodeEditor::toFront(QtNode *node)
{
	if (!node) {
		return;
	}

	const auto &items = m_graphics_scene->items();

	/* First set the node in front of all other nodes */
	for (const auto &item : items) {
		if (node != item && is_node(item) && item->isVisible()) {
			item->stackBefore(node);
		}
	}

	/* Put the connections of the node in front of the node and the other connections behind the node */
	for (const auto &item : items) {
		if (!is_node(item)) {
			continue;
		}

		const auto &connection = static_cast<QtConnection *>(item);

		if (node->isConnectionConnectedToThisNode(connection)) {
			node->stackBefore(item);
		}
		else {
			item->stackBefore(node);
		}
	}
}

void QtNodeEditor::toBack(QtNode *node)
{
	if (!node) {
		return;
	}

	const auto &items = m_graphics_scene->items();

	/* Set all other nodes in front of this node */
	for (const auto &item : items) {
		if (node != item && is_node(item) && item->isVisible()) {
			node->stackBefore(item);
		}
	}
}

void QtNodeEditor::setZoom(qreal zoom)
{
	m_view->scale(zoom, zoom);
}

bool QtNodeEditor::ctrlPressed()
{
	return (QGuiApplication::keyboardModifiers() & Qt::ControlModifier);
}

bool QtNodeEditor::isAlreadySelected(QtNode *node)
{
	auto iter = std::find(m_selected_nodes.begin(),
	                      m_selected_nodes.end(),
	                      node);

	return (iter != m_selected_nodes.end());
}

bool QtNodeEditor::isAlreadySelected(QtConnection *connection)
{
	auto iter = std::find(m_selected_connections.begin(),
	                      m_selected_connections.end(),
	                      connection);

	return (iter != m_selected_connections.end());
}

void QtNodeEditor::showContextMenu(const QPoint &pos)
{
	if (!m_context_menu) {
		return;
	}

	m_gestionnaire->ajourne_menu("Éditeur Noeud");

	m_context_menu->popup(pos);
}

void QtNodeEditor::update_state(event_type event)
{
	if (event == static_cast<event_type>(-1)) {
		return;
	}

	/* Clear all the node in the scene. */
	m_graphics_scene->clear();
	m_graphics_scene->items().clear();
	assert(m_graphics_scene->items().size() == 0);
	m_selected_nodes.clear();
	m_selected_connections.clear();

	/* Add nodes to the scene. */

	/* Add the object's graph's nodes to the scene. */
	if (m_context->eval_ctx->edit_mode) {
		auto scene_node = m_context->scene->active_node();

		if (scene_node == nullptr) {
			return;
		}

		std::unordered_map<Noeud *, QtNode *> node_items_map;

		auto object = static_cast<Object *>(scene_node);
		auto graph = object->graph();

		/* Add the nodes. */
		for (const auto &noeud : graph->noeuds()) {
			Noeud *pointeur_noeud = noeud.get();

			auto node_item = new QtNode(pointeur_noeud->nom().c_str());
			node_item->setTitleColor(Qt::white);
			node_item->alignTitle(ALIGNED_LEFT);
			node_item->pointeur_noeud(pointeur_noeud);
			node_item->setScene(m_graphics_scene);
			node_item->setEditor(this);
			node_item->setPos(pointeur_noeud->posx(), pointeur_noeud->posy());

			node_items_map[pointeur_noeud] = node_item;

			m_graphics_scene->addItem(node_item);

			if (pointeur_noeud->a_drapeau(NOEUD_SELECTIONE)) {
				m_selected_nodes.append(node_item);
			}
		}

		/* Add the connections. */
		for (const auto &lien : graph->liens()) {
			PriseSortie *sortie = lien->sortie;
			PriseEntree *entree = lien->entree;

			QtNode *from_node_item = node_items_map[sortie->parent];
			QtNode *to_node_item = node_items_map[entree->parent];

			QtPort *from_port = from_node_item->output(sortie->nom.c_str());
			QtPort *to_port = to_node_item->input(entree->nom.c_str());

			assert(from_port != nullptr && to_port != nullptr);

			from_node_item->createActiveConnection(from_port, from_port->pos());
			auto item = to_port->createConnection(from_node_item->m_active_connection);

			item->pointeur_lien(lien);

			if (lien->a_drapeau(NOEUD_SELECTIONE)) {
				item->setSelected(true);
			}

			from_node_item->m_active_connection = nullptr;
		}

		/* Add the children of this object. */
	}
	/* Add the object nodes to the scene. */
	else {
		for (const auto &node : m_context->scene->nodes()) {
			auto object = static_cast<Object *>(node.get());

			if (!object) {
				continue;
			}

			/* If it is has a parent, skip. */
			if (object->parent()) {
				continue;
			}

			auto node_item = new ObjectNodeItem(node.get(), node->name().c_str());
			node_item->setTitleColor(Qt::white);
			node_item->alignTitle(ALIGNED_CENTER);

			node_item->setEditor(this);
			node_item->setScene(m_graphics_scene);
			node_item->setPos(node->xpos(), node->ypos());

			if (node.get() == m_context->scene->active_node()) {
				m_selected_nodes.append(node_item);
			}

			m_graphics_scene->addItem(node_item);
		}
	}

	/* Make sure selected items are highlighted and in front of others. */
	for (auto node_item : m_selected_nodes) {
		node_item->setSelected(true);
		toFront(node_item);
	}
}

void QtNodeEditor::setActiveObject(ObjectNodeItem *node)
{
	m_context->scene->set_active_node(node->scene_node());
}

void QtNodeEditor::nodesConnected(QtNode *from, const QString &socket_from, QtNode *to, const QString &socket_to, bool notify)
{
	auto scene = m_context->scene;

	if (m_context->eval_ctx->edit_mode) {
		auto object = static_cast<Object *>(scene->active_node());
		auto graph = object->graph();

		auto noeud_de = from->pointeur_noeud();
		auto noeud_a = to->pointeur_noeud();

		auto prise_sortie = noeud_de->sortie(socket_from.toStdString());
		auto prise_entree = noeud_a->entree(socket_to.toStdString());

		assert((prise_sortie != nullptr) && (prise_entree != nullptr));

		graph->connecte(prise_sortie, prise_entree);

		/* Needed to prevent updating needlessly the graph when dropping a node on
		 * a connection. */
		if (notify) {
			scene->evalObjectDag(*m_context, object);
			scene->notify_listeners(event_type::node | event_type::modified);
		}
	}
	else {
		auto node_from = static_cast<ObjectNodeItem *>(from)->scene_node();
		auto node_to = static_cast<ObjectNodeItem *>(to)->scene_node();

		scene->connect(*m_context, node_from, node_to);
		scene->notify_listeners(event_type::object | event_type::parented);
	}
}

void QtNodeEditor::connectionRemoved(QtNode *from, const QString &socket_from, QtNode *to, const QString &socket_to, bool notify)
{
	auto scene = m_context->scene;

	if (m_context->eval_ctx->edit_mode) {
		auto object = static_cast<Object *>(scene->active_node());
		auto graph = object->graph();

		auto noeud_de = from->pointeur_noeud();
		auto noeud_a = to->pointeur_noeud();

		auto prise_sortie = noeud_de->sortie(socket_from.toStdString());
		auto prise_entree = noeud_a->entree(socket_to.toStdString());

		assert((prise_sortie != nullptr) && (prise_entree != nullptr));

		graph->deconnecte(prise_sortie, prise_entree);

		if (notify) {
			scene->evalObjectDag(*m_context, object);
			scene->notify_listeners(event_type::node | event_type::modified);
		}
	}
	else {
		auto node_from = static_cast<ObjectNodeItem *>(from)->scene_node();
		auto node_to = static_cast<ObjectNodeItem *>(to)->scene_node();

		scene->disconnect(*m_context, node_from, node_to);
		scene->notify_listeners(event_type::object | event_type::parented);
	}
}

void QtNodeEditor::sendNotification() const
{
	auto scene = m_context->scene;
	scene->notify_listeners(event_type::node | event_type::modified);
}

/* ************************************************************************** */

NodeView::NodeView(QWidget *parent)
    : QGraphicsView(parent)
{
	setDragMode(QGraphicsView::ScrollHandDrag);
}

NodeView::NodeView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
	setDragMode(QGraphicsView::ScrollHandDrag);
}

void NodeView::wheelEvent(QWheelEvent *event)
{
	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

	const auto factor = 1.15f;
	const auto zoom = ((event->delta() > 0) ? factor : 1.0f / factor);

	this->scale(zoom, zoom);
}

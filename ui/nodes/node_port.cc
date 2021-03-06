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

#include "node_port.h"

#include <iostream>

#include "node_constants.h"
#include "node_node.h"

static constexpr auto NODE_PORT_PEN_SIZE = 1;

QtPort::QtPort(const QString &portName,
               int portType,
               const QColor &portColour,
               const QColor &connectionColour,
               Alignment alignment,
               QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
    , m_port_name(portName)
    , m_port_type(portType)
    , m_port_colour(portColour)
    , m_connection_colour(connectionColour)
    , m_alignment(alignment)
    , m_label(new QGraphicsTextItem(this))
{
	m_label->setPlainText(portName);

	auto pen = QPen{m_port_colour};
	pen.setWidth(NODE_PORT_PEN_SIZE);
	setPen(pen);
	redraw();
}

void QtPort::setPortOpen(bool open)
{
	setBrush(open ? QColor("#000000ff") : m_port_colour);
}

void QtPort::setNameColor(const QColor &color)
{
	m_label->setDefaultTextColor(color);
}

void QtPort::redraw()
{
	m_font.setPointSize(NODE_PORT_FONT_SIZE);
	m_label->setFont(m_font);

	auto pen = QPen{m_port_colour};
	pen.setWidth(NODE_PORT_PEN_SIZE);
	setPen(pen);

	const auto &shapeSize = NODE_PORT_SHAPE_SIZE;

	auto p = QPainterPath{};
	p.addEllipse(-0.5f * shapeSize, -0.5f * shapeSize, shapeSize, shapeSize);
	setPath(p);

	setAlignedPos(pos());
}

qreal QtPort::getNormalizedWidth()
{
	/* Set the fontsize of the text to zoom = 1 and get the boundingRect().width() of the text and the shape */
	m_font.setPointSize(NODE_PORT_FONT_SIZE);
	m_label->setFont(m_font);
	auto width = m_label->boundingRect().width() + NODE_PORT_SHAPE_SIZE + NODE_PORT_WIDTH_MARGIN;
	m_font.setPointSize(NODE_PORT_FONT_SIZE);
	m_label->setFont(m_font);

	return width;
}

qreal QtPort::getNormalizedHeight()
{
	/* Set the fontsize of the text to zoom = 1 and get the boundingRect().heigth() of the text and the shape */
	m_font.setPointSize(NODE_PORT_FONT_SIZE);
	m_label->setFont(m_font);
	auto height = m_label->boundingRect().height();

	if (height < NODE_PORT_SHAPE_SIZE) {
		height = NODE_PORT_SHAPE_SIZE; /* In case the text is smaller than the port shape */
	}

	m_font.setPointSize(NODE_PORT_FONT_SIZE);
	m_label->setFont(m_font);

	return height;
}

void QtPort::setAlignedPos(const QPointF &pos)
{
	if (m_alignment == ALIGNED_RIGHT) {
		m_label->setPos(-m_label->boundingRect().width() - NODE_PORT_WIDTH_MARGIN, -0.5 * m_label->boundingRect().height());
	}
	else {
		m_label->setPos(NODE_PORT_WIDTH_MARGIN, -0.5 * m_label->boundingRect().height());
	}

	setPos(pos);
}

void QtPort::setAlignedPos(qreal x, qreal y)
{
	setAlignedPos(QPointF(x, y));
}

QtConnection *QtPort::createConnection(QtConnection *targetConnection)
{
	if (targetConnection) {
		/* The port is not the base, but the target */
		m_connections.push_back(targetConnection);
		targetConnection->setTargetPort(this);
	}
	else {
		/* The port is the base. Note that adding the connection to the scene is
		 * done on a higher level. */

		/* Don't make it a child of port, but do provide the port as a baseport */
		auto connection = new QtConnection(this);
		connection->setData(NODE_KEY_GRAPHIC_ITEM_TYPE, QVariant(NODE_VALUE_TYPE_CONNECTION));
		connection->setColor(m_connection_colour);

		m_connections.push_back(connection);
	}

	setPortOpen(false);
	return m_connections.back();
}

void QtPort::deleteConnection(QtConnection *connection, bool erase)
{
	if (!connection) {
		return;
	}

	const auto &base = connection->getBasePort();
	const auto &target = connection->getTargetPort();

	if (erase) {
		auto iter = std::find(m_connections.begin(), m_connections.end(), connection);
		m_connections.erase(iter);
	}

	if (base == this) {
		/* Inform the target port */
		if (target) {
			target->informConnectionDeleted(connection);
		}

		delete connection;
	}
	else {
		/* 'This' port is the target port; call the base port to delete its connection */
		base->deleteConnection(connection);
	}

	setPortOpen(m_connections.empty());
}

void QtPort::informConnectionDeleted(QtConnection *connection)
{
	auto iter = std::find(m_connections.begin(), m_connections.end(), connection);

	if (iter == m_connections.end()) {
		std::cerr << "Trying to delete connection not present in port!\n";
		return;
	}

	m_connections.erase(iter);

	setPortOpen(m_connections.size() == 0);
}

void QtPort::deleteAllConnections()
{
	for (auto &connection : m_connections) {
		deleteConnection(connection, false);
	}

	m_connections.clear();
}

void QtPort::updateConnection(const QPointF &altTargetPos)
{
	for (auto &connection : m_connections) {
		connection->updatePath(altTargetPos);
	}
}

void QtPort::collapse()
{
	setVisible(false);
	m_original_pos = pos();
	auto y = 0.5 * parentItem()->boundingRect().height();
	setAlignedPos(m_original_pos.x(), y);

	for (auto &connection : m_connections) {
		connection->setZValue(-1);
		connection->updatePath(pos());
	}
}

void QtPort::expand()
{
	setVisible(true);
	setAlignedPos(m_original_pos);

	for (auto &connection : m_connections) {
		connection->setZValue(1);
		connection->updatePath(pos());
	}
}

QVector<QtConnection *> &QtPort::getConnections()
{
	return m_connections;
}

const QVector<QtConnection *> &QtPort::getConnections() const
{
	return m_connections;
}

const QString &QtPort::getPortName() const
{
	return m_port_name;
}

const Alignment &QtPort::getAlignment() const
{
	return m_alignment;
}

int QtPort::getPortType() const
{
	return m_port_type;
}

bool QtPort::isPortOpen() const
{
	return !isConnected();
}

bool QtPort::isOutputPort() const
{
	return getPortType() == NODE_PORT_TYPE_OUTPUT;
}

bool QtPort::isConnected() const
{
	return !m_connections.empty();
}

bool is_connection_allowed(QtPort *from, QtPort *to)
{
	return from->getPortType() != to->getPortType();
}

#include "Node.h"

#include "Schematic.h"

using namespace AxiomModel;

Node::Node(Schematic *parent) : parent(parent) {

}

bool Node::isDragAvailable(QPoint delta) {
    return parent->positionAvailable(dragStartPos + delta, m_size, this);
}

void Node::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}

void Node::setPos(QPoint pos) {
    setPos(pos, true, true);
}

void Node::setSize(QSize size) {
    if (size != m_size) {
        if (size.width() < 1 || size.height() < 1 || !parent->positionAvailable(m_pos, size, this)) return;

        parent->freeGridRect(m_pos, m_size);
        emit beforeSizeChanged(size);
        parent->setGridRect(m_pos, size, this);
        m_size = size;
        emit sizeChanged(size);
    }
}

void Node::setCorners(QPoint topLeft, QPoint bottomRight) {
    auto newSize = QSize(bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y());
    if (newSize.width() < 1 || newSize.height() < 1) return;
    if (topLeft == m_pos && newSize == m_size) return;

    if (!parent->positionAvailable(topLeft, newSize, this)) {
        // try resizing just horizontally or just vertically
        auto hTopLeft = QPoint(topLeft.x(), m_pos.y());
        auto hSize = QSize(newSize.width(), m_size.height());
        auto vTopLeft = QPoint(m_pos.x(), topLeft.y());
        auto vSize = QSize(m_size.width(), newSize.height());
        if (parent->positionAvailable(hTopLeft, hSize, this)) {
            topLeft = hTopLeft;
            newSize = hSize;
        } else if (parent->positionAvailable(vTopLeft, vSize, this)) {
            topLeft = vTopLeft;
            newSize = vSize;
        }
    }

    if (topLeft == m_pos && newSize == m_size) return;
    parent->freeGridRect(m_pos, m_size);

    parent->setGridRect(topLeft, newSize, this);
    emit beforePosChanged(topLeft);
    m_pos = topLeft;
    emit posChanged(m_pos);
    emit beforeSizeChanged(newSize);
    m_size = newSize;
    emit sizeChanged(m_size);
}

void Node::select(bool exclusive) {
    if (exclusive || !m_selected) {
        m_selected = true;
        emit selected(exclusive);
    }
}

void Node::deselect() {
    if (!m_selected) return;
    m_selected = false;
    emit deselected();
}

void Node::startDragging() {
    dragStartPos = m_pos;
}

void Node::dragTo(QPoint delta) {
    setPos(dragStartPos + delta, false, false);
}

void Node::finishDragging() {
}

void Node::remove() {
    emit removed();
}

void Node::setPos(QPoint pos, bool updateGrid, bool checkPositions) {
    if (pos != m_pos) {
        if (checkPositions && !parent->positionAvailable(pos, m_size, this)) return;

        if (pos == m_pos) return;
        if (updateGrid) parent->freeGridRect(m_pos, m_size);

        emit beforePosChanged(pos);
        if (updateGrid) parent->setGridRect(pos, m_size, this);
        m_pos = pos;
        emit posChanged(pos);
    }
}

#include "Node.h"

#include "Schematic.h"

using namespace AxiomModel;

Node::Node(Schematic *parent) : parent(parent) {
}

void Node::setName(const QString &name) {
    if (name != m_name) {
        m_name = name;
        emit nameChanged(name);
    }
}

void Node::setPos(QPoint pos) {
    if (pos != m_pos) {
        auto newPos = m_pos;
        if (parent->positionAvailable(QPoint(pos.x(), newPos.y()), m_size, this)) {
            newPos.setX(pos.x());
        }
        if (parent->positionAvailable(QPoint(newPos.x(), pos.y()), m_size, this)) {
            newPos.setY(pos.y());
        }
        if (newPos == m_pos) return;

        parent->freeGridRect(m_pos, m_size);

        emit beforePosChanged(newPos);
        parent->setGridRect(newPos, m_size, this);
        m_pos = newPos;
        emit posChanged(newPos);
    }
}

void Node::setSize(QSize size) {
    if (size != m_size) {
        parent->positionAvailable(m_pos, &size, this);
        if (size.width() < 1 || size.height() < 1) return;

        parent->freeGridRect(m_pos, m_size);
        emit beforeSizeChanged(size);
        parent->setGridRect(m_pos, size, this);
        m_size = size;
        emit sizeChanged(size);
    }
}
}

void Node::setSelected(bool selected) {
    if (selected != m_selected) {
        m_selected = selected;
        emit selectedChanged(selected);
    }
}

void Node::remove() {
    parent->removeNode(this);
    emit removed();
}

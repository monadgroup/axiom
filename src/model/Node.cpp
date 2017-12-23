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
        if (!parent->positionAvailable(pos, m_size, this)) return;
        parent->freeGridRect(m_pos, m_size);

        emit beforePosChanged(pos);
        parent->setGridRect(pos, m_size, this);
        m_pos = pos;
        emit posChanged(pos);
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

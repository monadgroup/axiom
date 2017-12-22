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
        m_pos = pos;
        emit posChanged(pos);
    }
}

void Node::setSize(QSize size) {
    if (size != m_size) {
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

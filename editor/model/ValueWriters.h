#pragma once

#include <QtCore/QDataStream>
#include "compiler/runtime/ValueOperator.h"

namespace AxiomModel {

    QDataStream &operator<<(QDataStream &stream, const MaximRuntime::NumValue &val);

    QDataStream &operator>>(QDataStream &stream, MaximRuntime::NumValue &val);

    QDataStream &operator<<(QDataStream &stream, const MaximRuntime::MidiEventValue &val);

    QDataStream &operator>>(QDataStream &stream, MaximRuntime::MidiEventValue &val);

    QDataStream &operator<<(QDataStream &stream, const MaximRuntime::MidiValue &val);

    QDataStream &operator>>(QDataStream &stream, MaximRuntime::MidiValue &val);

}

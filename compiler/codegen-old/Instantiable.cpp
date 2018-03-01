#include "Instantiable.h"

using namespace MaximCodegen;

size_t Instantiable::_nextId = 0;

Instantiable::Instantiable() : _id(_nextId++) {

}

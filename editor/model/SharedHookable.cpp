#include "SharedHookable.h"

#include <vector>

using namespace AxiomModel;

SharedHookable::SharedHookable() : impl(std::make_shared<HookContext>()) {

}

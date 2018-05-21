#include "SharedHookable.h"

#include <vector>

using namespace AxiomCommon;

SharedHookable::SharedHookable() : impl(std::make_shared<HookContext>()) {

}

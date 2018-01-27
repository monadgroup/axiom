#pragma once

namespace MaximUtil {

    template<typename Derived, typename Base>
    std::unique_ptr<Derived> dynamic_unique_cast(std::unique_ptr<Base> &&p) {
        if (Derived *result = dynamic_cast<Derived*>(p.get())) {
            p.release();
            return std::unique_ptr<Derived>(result);
        }
        return std::unique_ptr<Derived>(nullptr);
    };

}

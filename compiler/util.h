#pragma once

namespace MaximUtil {

    template <typename To, typename From, typename Deleter>
    std::unique_ptr<To, Deleter> dynamic_unique_cast(std::unique_ptr<From, Deleter> &&p) {
        if (To *cast = dynamic_cast<To*>(p.get())) {
            std::unique_ptr<To, Deleter> result(cast, std::move(p.get_deleter()));
            p.release();
            return result;
        }
        return std::unique_ptr<To, Deleter>(nullptr);
    };

}

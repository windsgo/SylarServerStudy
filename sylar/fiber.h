#pragma once

#include <ucontext.h>
#include <memory>

#include "thread.h"

namespace sylar
{

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;

private:
};

} // namespace sylar

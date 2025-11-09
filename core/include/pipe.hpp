#pragma once

#include <input.hpp>
#include <output.hpp>

#include <memory>

namespace coreutils {

std::pair<std::unique_ptr<Input>, std::unique_ptr<Output>> createPipe();

}  // namespace coreutils

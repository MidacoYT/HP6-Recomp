#include "presentation/PresentationFocusRegistry.h"

#include "math/Transform.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace hp6::presentation {

PresentationMatrix make_presentation_focus_follower_local(
    const PresentationMatrix& authored_matrix) {
    PresentationMatrix orthonormal = authored_matrix;
    const auto normalize = [&](int row) {
        const float length = std::sqrt(
            orthonormal[row] * orthonormal[row] +
            orthonormal[row + 1] * orthonormal[row + 1] +
            orthonormal[row + 2] * orthonormal[row + 2]);
        if (length == 0.0f) {
            throw std::runtime_error("degenerate presentation follower basis");
        }
        for (int axis = 0; axis < 3; ++axis) {
            orthonormal[row + axis] /= length;
        }
        orthonormal[row + 3] = 0.0f;
    };
    normalize(8);
    const float projection = orthonormal[4] * orthonormal[8] +
        orthonormal[5] * orthonormal[9] +
        orthonormal[6] * orthonormal[10];
    for (int axis = 0; axis < 3; ++axis) {
        orthonormal[4 + axis] -= projection * orthonormal[8 + axis];
    }
    normalize(4);
    for (int axis = 0; axis < 3; ++axis) {
        orthonormal[axis] =
            orthonormal[4 + (axis + 1) % 3] *
                orthonormal[8 + (axis + 2) % 3] -
            orthonormal[4 + (axis + 2) % 3] *
                orthonormal[8 + (axis + 1) % 3];
    }
    orthonormal[3] = 0.0f;
    PresentationMatrix inverse{};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            inverse[row * 4 + column] = orthonormal[column * 4 + row];
        }
    }
    for (int column = 0; column < 3; ++column) {
        inverse[12 + column] = -(
            orthonormal[12] * inverse[column] +
            orthonormal[13] * inverse[4 + column] +
            orthonormal[14] * inverse[8 + column]);
    }
    inverse[15] = 1.0f;
    return inverse;
}

void PresentationFocusRegistry::publish(
    std::string_view name, const PresentationMatrix& matrix) {
    matrices_.insert_or_assign(std::string(name), matrix);
}

void PresentationFocusRegistry::remove(std::string_view name) {
    matrices_.erase(std::string(name));
}

void PresentationFocusRegistry::remove_alias(std::string_view alias) {
    aliases_.erase(std::string(alias));
}

void PresentationFocusRegistry::set_alias(std::string_view alias,
                                          std::string_view target) {
    aliases_.insert_or_assign(std::string(alias), std::string(target));
}

PresentationFocusLocatorMovable::PresentationFocusLocatorMovable(
    PresentationFocusRegistry& registry, std::string name,
    const PresentationMatrix& pose)
    : registry_(registry), name_(std::move(name)), pose_(pose) {}

void PresentationFocusLocatorMovable::publish_pose() {
    registry_.remove_alias(name_);
    registry_.publish(name_, pose_);
}

void PresentationFocusLocatorMovable::transition_to(int state) {
    state_ = state;
    if (state == 6) publish_pose();
}

void PresentationFocusLocatorMovable::update(const PresentationMatrix& pose) {
    pose_ = pose;
    if (state_ >= 6) publish_pose();
}

bool PresentationFocusPhase::request_focus() {
    if (state_ == 4 || state_ == 6) return true;
    if (state_ != 3) return false;
    state_ = 4;
    locator_.transition_to(state_);
    return true;
}

bool PresentationFocusPhase::request_blur() {
    if (state_ == 3 || state_ == 5) return true;
    if (state_ != 6) return false;
    state_ = 5;
    locator_.transition_to(state_);
    return true;
}

bool PresentationFocusPhase::tick(bool transition_complete) {
    if (!transition_complete) return false;
    if (state_ == 4) state_ = 6;
    else if (state_ == 5) state_ = 3;
    else return false;
    locator_.transition_to(state_);
    return true;
}

bool PresentationFocusRegistry::lookup(
    std::string_view name, PresentationMatrix& output) const {
    const auto found = matrices_.find(std::string(name));
    if (found != matrices_.end()) {
        output = found->second;
        return true;
    }
    const auto alias = aliases_.find(std::string(name));
    if (alias == aliases_.end()) return false;
    const auto target = matrices_.find(alias->second);
    if (target == matrices_.end()) return false;
    output = target->second;
    return true;
}

PresentationMatrix compose_presentation_focus_follower(
    const PresentationFocusRegistry& registry, std::string_view locator_name,
    const PresentationMatrix& local_matrix) {
    PresentationMatrix located = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
    registry.lookup(locator_name, located);
    PresentationMatrix result{};
    hp6::math::multiply_matrix4x4(result.data(), local_matrix.data(),
                                  located.data());
    return result;
}

} // namespace hp6::presentation

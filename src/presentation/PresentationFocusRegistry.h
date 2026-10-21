#pragma once

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>

namespace hp6::presentation {

using PresentationMatrix = std::array<float, 16>;

// Observable contract of 0x005d3570 (publish), 0x005d3730 (lookup), and
// 0x005d3800 (alias fallback). Matrix storage is owned by this registry.
class PresentationFocusRegistry {
public:
    void publish(std::string_view name, const PresentationMatrix& matrix);
    void remove(std::string_view name);
    void remove_alias(std::string_view alias);
    void set_alias(std::string_view alias, std::string_view target);
    bool lookup(std::string_view name, PresentationMatrix& output) const;

private:
    std::unordered_map<std::string, PresentationMatrix> matrices_;
    std::unordered_map<std::string, std::string> aliases_;
};

// 0x004ea9e0 publishes when a focus locator enters state 6; 0x004eaa20
// republishes its current pose during updates while state >= 6. Both clear
// the name's alias before publishing. Teardown of the locator subobject does
// not itself remove the named transform from the shared manager.
class PresentationFocusLocatorMovable {
public:
    PresentationFocusLocatorMovable(PresentationFocusRegistry& registry,
                                    std::string name,
                                    const PresentationMatrix& pose);
    PresentationFocusLocatorMovable(const PresentationFocusLocatorMovable&) = delete;
    PresentationFocusLocatorMovable& operator=(const PresentationFocusLocatorMovable&) = delete;

    void transition_to(int state);
    void update(const PresentationMatrix& pose);

private:
    PresentationFocusRegistry& registry_;
    std::string name_;
    PresentationMatrix pose_{};
    int state_ = 0;
    void publish_pose();
};

// Focus subset of the interface state machine at 0x0043b5d0. The menu has
// already brought each button to stable state 3 before selecting one. Request
// 3 starts focus (4), request 4 starts blur (5), and request 0 completes either
// transition only when the owner's completion callback reports true.
class PresentationFocusPhase {
public:
    explicit PresentationFocusPhase(PresentationFocusLocatorMovable& locator)
        : locator_(locator) {}

    bool request_focus();
    bool request_blur();
    bool tick(bool transition_complete);
    int state() const { return state_; }

private:
    PresentationFocusLocatorMovable& locator_;
    int state_ = 3;
};

// 0x005c5f40 calls 0x00404970 to orthonormalize the authored pose and
// 0x00431910 to store its inverse at follower+0x20.
PresentationMatrix make_presentation_focus_follower_local(
    const PresentationMatrix& authored_matrix);

// 0x005c6030 initializes an identity matrix, resolves the follower's name,
// then uses 0x004048f0 -> 0x00401520: local * located matrix.
PresentationMatrix compose_presentation_focus_follower(
    const PresentationFocusRegistry& registry, std::string_view locator_name,
    const PresentationMatrix& local_matrix);

} // namespace hp6::presentation

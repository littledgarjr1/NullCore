#include <Geode/Geode.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/loader/SettingV3.hpp>
#include "settings.hpp"

using namespace geode::prelude;

// NullCore feature keybinds are registered as normal Geode settings so users
// can customize them from the Geode mod settings interface.
$on_game(Loaded) {
    listenForKeybindSettingPresses(
        "noclip-keybind",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) {
                nc::set("noclip", !nc::get("noclip"));
            }
        }
    );

    listenForKeybindSettingPresses(
        "auto-clicker-keybind",
        [](Keybind const&, bool down, bool repeat, double) {
            if (down && !repeat) {
                nc::set("auto-clicker", !nc::get("auto-clicker"));
            }
        }
    );
}

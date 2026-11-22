#pragma once

namespace nc::updater {
    // Looks for a newer build on GitHub. manual = true also reports "up to date" and errors.
    void check(bool manual);
}

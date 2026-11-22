#include <Geode/Geode.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/ui/Notification.hpp>
#include "updater.hpp"

using namespace geode::prelude;

#ifndef NULLCORE_GIT_SHA
#define NULLCORE_GIT_SHA "unknown"
#endif

namespace {
    constexpr char const* RELEASE_URL =
        "https://api.github.com/repos/littledgarjr1/NullCore/releases/tags/nightly";

    geode::async::TaskHolder<web::WebResponse> s_checkTask;
    geode::async::TaskHolder<web::WebResponse> s_downloadTask;

    void say(std::string const& text) {
        FLAlertLayer::create("NullCore", text, "OK")->show();
    }

    void install(web::WebResponse res) {
        if (!res.ok()) {
            say(fmt::format("Download failed (code {}).", res.code()));
            return;
        }
        auto const& data = res.data();
        // A .geode file is a zip, so it must start with "PK".
        if (data.size() < 1024 || data[0] != 'P' || data[1] != 'K') {
            say("The downloaded file doesn't look like a valid .geode file.");
            return;
        }

        auto target = dirs::getModsDir() / fmt::format("{}.geode", Mod::get()->getID());
        std::error_code ec;
        std::filesystem::remove(Mod::get()->getPackagePath(), ec); // same approach Geode itself uses
        auto ok = file::writeBinary(target, data);
        if (!ok) {
            say(fmt::format("Couldn't save the update: {}", ok.unwrapErr()));
            return;
        }
        say("Update installed!\nFully close and reopen Geometry Dash to use the new version.");
    }

    void startDownload(std::string url) {
        Notification::create("Downloading NullCore update...", NotificationIcon::Loading, 2.f)->show();
        s_downloadTask.spawn(
            web::WebRequest()
                .userAgent("NullCore-Updater")
                .followRedirects(true)
                .timeout(std::chrono::seconds(120))
                .get(url),
            [](web::WebResponse res) { install(std::move(res)); }
        );
    }

    void onRelease(web::WebResponse res, bool manual, std::string current) {
        if (!res.ok()) {
            log::warn("NullCore: update check failed (code {})", res.code());
            if (manual) say(fmt::format("Couldn't reach GitHub (code {}).", res.code()));
            return;
        }
        auto parsed = res.json();
        if (!parsed) {
            if (manual) say("Couldn't read the update info from GitHub.");
            return;
        }
        auto json = std::move(parsed).unwrap();

        // The release body is written by the workflow as "Build of <commit sha>".
        std::string body = json["body"].asString().unwrapOr("");
        std::string latest;
        auto pos = body.find("Build of ");
        if (pos != std::string::npos) latest = body.substr(pos + 9, 40);
        if (latest.empty()) latest = json["target_commitish"].asString().unwrapOr("");

        if (latest.empty()) {
            if (manual) say("The latest release has no build info.");
            return;
        }
        if (latest == current) {
            if (manual) say("You're on the latest NullCore build.");
            return;
        }

        std::string url;
        auto assets = json["assets"].asArray();
        if (assets) {
            for (auto a : assets.unwrap()) {
                auto name = a["name"].asString().unwrapOr("");
                if (name.ends_with(".geode")) {
                    url = a["browser_download_url"].asString().unwrapOr("");
                    break;
                }
            }
        }
        if (url.empty()) {
            if (manual) say("A newer build exists, but it has no .geode file attached.");
            return;
        }

        createQuickPopup(
            "NullCore update",
            "A newer NullCore build is available.\nInstall it now? You'll need to restart the game afterwards.",
            "Later", "Install",
            [url](FLAlertLayer*, bool install) {
                if (install) startDownload(url);
            }
        );
    }
}

void nc::updater::check(bool manual) {
    std::string current = NULLCORE_GIT_SHA;
    if (current == "unknown") {
        if (manual) say("This build has no version info (it wasn't built by the GitHub workflow), so it can't check for updates.");
        return;
    }
    s_checkTask.spawn(
        web::WebRequest()
            .userAgent("NullCore-Updater")
            .followRedirects(true)
            .timeout(std::chrono::seconds(20))
            .get(RELEASE_URL),
        [manual, current](web::WebResponse res) { onRelease(std::move(res), manual, current); }
    );
}

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>
#include <cctype>
#include <filesystem>

using namespace geode::prelude;

namespace frameforge {
    struct InputEvent {
        uint64_t tick = 0;
        bool down = true;
        bool player1 = true;
        int button = 1;
    };

    struct Run {
        int tickRate = 240;
        std::string levelName;
        std::vector<InputEvent> inputs;
        uint64_t elapsedTicks = 0;
    };

    static Run g_run;
    static bool g_recording = false;
    static bool g_haveRun = false;
    static uint64_t g_tick = 0;
    static std::string g_lastSaved;

    static std::filesystem::path savePath() {
        return std::filesystem::path(Mod::get()->getSaveDir()) / "recordings";
    }

    static std::string safeName(std::string value) {
        for (auto& c : value) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_')) c = '_';
        }
        if (value.empty()) value = "untitled";
        return value;
    }

    static void resetRun() {
        g_run = Run{};
        g_run.tickRate = Mod::get()->getSettingValue<int>("recording-fps");
        g_tick = 0;
        g_haveRun = false;
    }

    static void recordInput(bool down, bool player1, int button) {
        if (!g_recording) return;
        if (!down && !Mod::get()->getSettingValue<bool>("capture-releases")) return;
        g_run.inputs.push_back(InputEvent{g_tick, down, player1, button});
    }

    static std::string makeCsv() {
        std::ostringstream out;
        out << "# FrameForge recording v1\n";
        out << "# tick_rate," << g_run.tickRate << "\n";
        out << "# elapsed_ticks," << g_run.elapsedTicks << "\n";
        out << "# level," << safeName(g_run.levelName) << "\n";
        out << "tick,action,player,button\n";
        for (auto const& e : g_run.inputs) {
            out << e.tick << ',' << (e.down ? "press" : "release") << ','
                << (e.player1 ? 1 : 2) << ',' << e.button << '\n';
        }
        return out.str();
    }

    static bool saveRun() {
        std::error_code ec;
        auto dir = savePath();
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            log::error("FrameForge: could not create save directory: {}", ec.message());
            return false;
        }
        auto stamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        auto name = safeName(g_run.levelName) + "-" + std::to_string(stamp) + ".csv";
        auto path = dir / name;
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;
        file << makeCsv();
        g_lastSaved = path.string();
        log::info("FrameForge: saved {} inputs to {}", g_run.inputs.size(), g_lastSaved);
        return true;
    }
}

// The tick counter follows the game's update loop. It records game-update ticks,
// not monitor refreshes; use a compatible physics/TPS bypass for non-240 rates.
class $modify(FrameForgePlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreate) {
        if (!PlayLayer::init(level, useReplay, dontCreate)) return false;
        frameforge::resetRun();
        if (level) frameforge::g_run.levelName = level->m_levelName;
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (frameforge::g_recording) {
            ++frameforge::g_tick;
            frameforge::g_run.elapsedTicks = frameforge::g_tick;
        }
    }

    void onQuit() {
        if (frameforge::g_recording) {
            frameforge::g_recording = false;
            frameforge::g_haveRun = !frameforge::g_run.inputs.empty();
            if (frameforge::g_haveRun && Mod::get()->getSettingValue<bool>("auto-save")) {
                frameforge::saveRun();
            }
        }
        PlayLayer::onQuit();
    }
};

class $modify(FrameForgeInputLayer, GJBaseGameLayer) {
    void handleButton(bool down, bool player1, int button, bool isPlayer2) {
        frameforge::recordInput(down, player1, button);
        GJBaseGameLayer::handleButton(down, player1, button, isPlayer2);
    }
};

$on_mod(Loaded) {
    log::info("FrameForge loaded. Recording hooks are ready.");
}

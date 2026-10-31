#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>
#include <optional>
#include <string>
#include <vector>

using namespace geode::prelude;

struct Stop {
    float at = 0.f;
    ccColor3B color = {255, 255, 255};
};

namespace cfg {
    // index 0 = Player 1, 1 = Player 2
    inline std::vector<Stop> g_stops[2];
    inline bool g_enabled = true;

    inline std::string toHex(ccColor3B c) {
        return fmt::format("{:02X}{:02X}{:02X}", c.r, c.g, c.b);
    }

    inline std::optional<ccColor3B> fromHex(std::string s) {
        if (!s.empty() && s[0] == '#') s.erase(0, 1);
        if (s.size() != 6) return std::nullopt;
        auto r = geode::utils::numFromString<uint32_t>(s, 16);
        if (!r.isOk()) return std::nullopt;
        uint32_t v = r.unwrap();
        return ccColor3B{
            static_cast<GLubyte>((v >> 16) & 0xFF),
            static_cast<GLubyte>((v >> 8) & 0xFF),
            static_cast<GLubyte>(v & 0xFF)
        };
    }

    inline void sortStops(std::vector<Stop>& v) {
        std::stable_sort(v.begin(), v.end(), [](Stop const& a, Stop const& b) {
            return a.at < b.at;
        });
    }

    // format: "0:FF0000;12.5:00FF00"
    inline std::string serialize(std::vector<Stop> const& v) {
        std::string out;
        for (auto const& s : v) {
            if (!out.empty()) out += ";";
            out += fmt::format("{}:{}", s.at, toHex(s.color));
        }
        return out;
    }

    inline std::vector<Stop> parse(std::string const& str) {
        std::vector<Stop> out;
        for (auto const& part : geode::utils::string::split(str, ";")) {
            auto pos = part.find(':');
            if (pos == std::string::npos) continue;
            auto at = geode::utils::numFromString<float>(part.substr(0, pos));
            auto col = fromHex(part.substr(pos + 1));
            if (!at.isOk() || !col) continue;
            out.push_back({at.unwrap(), *col});
        }
        sortStops(out);
        return out;
    }

    inline void load() {
        g_enabled = Mod::get()->getSavedValue<bool>("enabled", true);
        g_stops[0] = parse(Mod::get()->getSavedValue<std::string>("p1-stops", ""));
        g_stops[1] = parse(Mod::get()->getSavedValue<std::string>("p2-stops", ""));
    }

    inline void save() {
        Mod::get()->setSavedValue<bool>("enabled", g_enabled);
        Mod::get()->setSavedValue<std::string>("p1-stops", serialize(g_stops[0]));
        Mod::get()->setSavedValue<std::string>("p2-stops", serialize(g_stops[1]));
        (void)Mod::get()->saveData();
    }

    // Color for a given percent. nullopt = before the first stop -> keep the game's own (icon) color.
    inline std::optional<ccColor3B> colorAt(int player, float percent) {
        std::optional<ccColor3B> result;
        if (!g_enabled) return result;
        for (auto const& s : g_stops[player]) {
            if (s.at <= percent) result = s.color;
            else break;
        }
        return result;
    }
}

#include "Config.hpp"
#include "WaveTrailPopup.hpp"

#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

$on_mod(Loaded) {
    cfg::load();
}

// The color is a pure function of the current level percent, so:
//  - StartPos works (percent comes from the player's X position),
//  - leaving and re-entering the wave keeps the same colors,
//  - dying / restarting needs no extra state.
class $modify(PWTHardStreak, HardStreak) {
    void updateStroke(float dt) {
        if (auto pl = PlayLayer::get()) {
            int player = -1;
            if (pl->m_player1 && pl->m_player1->m_waveTrail == this) player = 0;
            else if (pl->m_player2 && pl->m_player2->m_waveTrail == this) player = 1;

            if (player >= 0) {
                if (auto c = cfg::colorAt(player, pl->getCurrentPercent())) {
                    this->setColor(*c);
                }
            }
        }
        HardStreak::updateStroke(dt);
    }
};

class $modify(PWTPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        if (!Mod::get()->getSettingValue<bool>("pause-button")) return;

        auto win = CCDirector::get()->getWinSize();
        auto menu = CCMenu::create();
        menu->setPosition({75.f, win.height - 25.f});

        auto btn = CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create("Wave Trail"),
            [](auto) { WaveTrailPopup::create()->show(); }
        );
        btn->setScale(0.7f);
        menu->addChild(btn);
        this->addChild(menu);
    }
};

class $modify(PWTMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto menu = CCMenu::create();
        menu->setPosition({75.f, 22.f});

        auto btn = CCMenuItemExt::createSpriteExtra(
            ButtonSprite::create("Wave Trail"),
            [](auto) { WaveTrailPopup::create()->show(); }
        );
        btn->setScale(0.7f);
        menu->addChild(btn);
        this->addChild(menu);
        return true;
    }
};

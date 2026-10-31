#pragma once
#include "Config.hpp"

class WaveTrailPopup : public geode::Popup {
protected:
    std::vector<Stop> m_stops[2];
    bool m_enabled = true;
    int m_player = 0;
    ScrollLayer* m_scroll = nullptr;
    CCLabelBMFont* m_playerLabel = nullptr;

    bool init();
    void rebuild();
    CCNode* makeRow(size_t index);
    void onApply();

public:
    static WaveTrailPopup* create();
};

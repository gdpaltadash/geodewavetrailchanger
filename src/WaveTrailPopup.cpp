#include "WaveTrailPopup.hpp"

WaveTrailPopup* WaveTrailPopup::create() {
    auto ret = new WaveTrailPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool WaveTrailPopup::init() {
    if (!Popup::init(380.f, 280.f)) return false;
    this->setTitle("Percent Wave Trail");

    // working copies: nothing is saved until Apply is pressed
    m_stops[0] = cfg::g_stops[0];
    m_stops[1] = cfg::g_stops[1];

    auto size = m_mainLayer->getContentSize();

    auto menu = CCMenu::create();
    menu->setPosition({0.f, 0.f});
    menu->setContentSize(size);
    m_mainLayer->addChild(menu);

    // player selector
    m_playerLabel = CCLabelBMFont::create("Player 1", "goldFont.fnt");
    m_playerLabel->setScale(0.7f);
    m_playerLabel->setPosition({110.f, size.height - 50.f});
    m_mainLayer->addChild(m_playerLabel);

    auto switchBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Switch player"),
        [this](auto) {
            m_player = 1 - m_player;
            m_playerLabel->setString(m_player == 0 ? "Player 1" : "Player 2");
            this->rebuild();
        }
    );
    switchBtn->setScale(0.7f);
    switchBtn->setPosition({270.f, size.height - 50.f});
    menu->addChild(switchBtn);

    auto hint = CCLabelBMFont::create(
        "Before the first row the wave keeps your icon color", "chatFont.fnt");
    hint->setScale(0.6f);
    hint->setOpacity(160);
    hint->setPosition({size.width / 2.f, size.height - 70.f});
    m_mainLayer->addChild(hint);

    // list
    auto bg = CCScale9Sprite::create("square02b_001.png", {0, 0, 80, 80});
    bg->setColor({0, 0, 0});
    bg->setOpacity(90);
    bg->setContentSize({350.f, 140.f});
    bg->setPosition({size.width / 2.f, 112.f});
    m_mainLayer->addChild(bg);

    m_scroll = ScrollLayer::create({340.f, 130.f});
    m_scroll->setPosition({size.width / 2.f - 170.f, 112.f - 65.f});
    m_scroll->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(m_scroll->getContentHeight())
            ->setGap(4.f)
    );
    m_mainLayer->addChild(m_scroll);

    // bottom buttons
    auto addBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Add row"),
        [this](auto) {
            auto& v = m_stops[m_player];
            float next = v.empty() ? 0.f : std::min(100.f, v.back().at + 10.f);
            v.push_back({next, ccColor3B{255, 255, 255}});
            this->rebuild();
            m_scroll->m_contentLayer->setPositionY(0.f); // jump to the newest row
        }
    );
    addBtn->setPosition({110.f, 22.f});
    menu->addChild(addBtn);

    auto applyBtn = CCMenuItemExt::createSpriteExtra(
        ButtonSprite::create("Apply"),
        [this](auto) { this->onApply(); }
    );
    applyBtn->setPosition({270.f, 22.f});
    menu->addChild(applyBtn);

    this->rebuild();
    return true;
}

void WaveTrailPopup::rebuild() {
    m_scroll->m_contentLayer->removeAllChildren();
    auto& v = m_stops[m_player];
    for (size_t i = 0; i < v.size(); i++) {
        m_scroll->m_contentLayer->addChild(this->makeRow(i));
    }
    m_scroll->m_contentLayer->updateLayout();
}

CCNode* WaveTrailPopup::makeRow(size_t index) {
    auto row = CCNode::create();
    row->setContentSize({340.f, 30.f});

    auto& stop = m_stops[m_player][index];

    // percent
    auto percentInput = TextInput::create(90.f, "%", "bigFont.fnt");
    percentInput->setFilter("0123456789.");
    percentInput->setMaxCharCount(7);
    percentInput->setString(fmt::format("{}", stop.at));
    percentInput->setPosition({55.f, 15.f});
    percentInput->setCallback([this, index](std::string const& text) {
        auto r = geode::utils::numFromString<float>(text);
        if (!r.isOk()) return;
        auto& v = m_stops[m_player];
        if (index < v.size()) v[index].at = std::clamp(r.unwrap(), 0.f, 100.f);
    });
    row->addChild(percentInput);

    // color preview
    auto swatch = CCLayerColor::create(ccColor4B{stop.color.r, stop.color.g, stop.color.b, 255}, 24.f, 24.f);
    swatch->setPosition({125.f, 3.f});
    row->addChild(swatch);

    // hex
    auto hexInput = TextInput::create(110.f, "RRGGBB", "bigFont.fnt");
    hexInput->setFilter("0123456789abcdefABCDEF#");
    hexInput->setMaxCharCount(7);
    hexInput->setString(cfg::toHex(stop.color));
    hexInput->setPosition({215.f, 15.f});
    hexInput->setCallback([this, index, swatch](std::string const& text) {
        auto c = cfg::fromHex(text);
        if (!c) return;
        auto& v = m_stops[m_player];
        if (index < v.size()) v[index].color = *c;
        swatch->setColor(*c);
    });
    row->addChild(hexInput);

    // delete
    auto rowMenu = CCMenu::create();
    rowMenu->setPosition({0.f, 0.f});
    rowMenu->setContentSize(row->getContentSize());
    row->addChild(rowMenu);

    auto trash = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    trash->setScale(0.6f);
    auto delBtn = CCMenuItemExt::createSpriteExtra(trash, [this, index](auto) {
        auto& v = m_stops[m_player];
        if (index < v.size()) v.erase(v.begin() + index);
        this->rebuild();
    });
    delBtn->setPosition({315.f, 15.f});
    rowMenu->addChild(delBtn);

    return row;
}

void WaveTrailPopup::onApply() {
    for (int i = 0; i < 2; i++) {
        cfg::sortStops(m_stops[i]);
        cfg::g_stops[i] = m_stops[i];
    }
    cfg::save();
    this->rebuild(); // shows the rows in sorted order
    Notification::create("Wave trail colors saved", NotificationIcon::Success)->show();
}

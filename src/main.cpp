#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

class multiGIDPopup : public geode::Popup<> {
protected:
  static constexpr float popupWidth = 220.f;
  static constexpr float popupHeight = 130.f;

  std::function<void()> m_onNewGroupCallback;
  std::function<void()> m_onNewGroupXCallback;
  std::function<void()> m_onNewGroupYCallback;

  bool setup() override {
    this->setTitle("Next Free GID");

    // New Group X and Y buttons
    auto btnNew = ButtonSprite::create("New Group", "bigFont.fnt",
                                       "GJ_button_01.png", 1.f);
    auto btnX = ButtonSprite::create("New Groups X", "bigFont.fnt",
                                     "GJ_button_01.png", 1.f);
    auto btnY = ButtonSprite::create("New Groups Y", "bigFont.fnt",
                                     "GJ_button_01.png", 1.f);

    btnNew->setScale(0.5f);
    btnX->setScale(0.5f);
    btnY->setScale(0.5f);

    auto newItem = CCMenuItemSpriteExtra::create(
        btnNew, this, menu_selector(multiGIDPopup::onNewGroup));
    auto xItem = CCMenuItemSpriteExtra::create(
        btnX, this, menu_selector(multiGIDPopup::onNewGroupX));
    auto yItem = CCMenuItemSpriteExtra::create(
        btnY, this, menu_selector(multiGIDPopup::onNewGroupY));

    auto buttonsMenu = CCMenu::create(newItem, xItem, yItem, nullptr);
    buttonsMenu->alignItemsVerticallyWithPadding(
        7.f); // vertical spacing between buttons
    buttonsMenu->setPosition(
        {popupWidth / 2.f, 55.f}); // adjust Y to position menu vertically
    m_mainLayer->addChild(buttonsMenu);

    // Help Button
    auto helpSprite =
        CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    helpSprite->setScale(0.7f);
    auto helpButton = CCMenuItemSpriteExtra::create(
        helpSprite, this, menu_selector(multiGIDPopup::onInfoClicked));

    auto helpMenu = CCMenu::create(helpButton, nullptr);
    helpMenu->setAnchorPoint({1.f, 1.f});
    helpMenu->setPosition({popupWidth - 12.f, popupHeight - 12.f});
    m_mainLayer->addChild(helpMenu);

    return true;
  }

  void onNewGroup(CCObject *sender) {
    log::info("New Group selected");
    if (m_onNewGroupCallback) {
      m_onNewGroupCallback(); // Call set new group individually
    }
    this->onClose(sender);
  }

  void onNewGroupX(CCObject *sender) {
    log::info("New Group X selected");
    if (m_onNewGroupXCallback) {
      m_onNewGroupXCallback(); // Call assignNewGroups(false) => New Group X
    }
    this->onClose(sender);
  }

  void onNewGroupY(CCObject *sender) {
    log::info("New Group Y selected");
    if (m_onNewGroupYCallback) {
      m_onNewGroupYCallback(); // Call assignNewGroups(true) => New Group Y
    }
    this->onClose(sender);
  }

  void onInfoClicked(CCObject *) {
    // Show explanation popup when "i" clicked
    FLAlertLayer::create(
        "Help", // title
        "<cy>New Group</c> assigns <cg>one</c> new group ID to all selected "
        "objects.\n"
        "<cy>New Groups X</c> assigns new group IDs to selected objects from "
        "<cg>left to right</c>.\n"
        "<cy>New Groups Y</c> assigns new group IDs to selected objects from "
        "<cg>bottom to top</c>.", // content
        "OK"                      // button
        )
        ->show();
  }

public:
  static multiGIDPopup *create(std::function<void()> onNewGroup,
                               std::function<void()> onNewGroupX,
                               std::function<void()> onNewGroupY) {
    auto ret = new multiGIDPopup(); // Pointer to popup
    if (ret && ret->initAnchored(popupWidth, popupHeight)) {
      ret->m_onNewGroupCallback = std::move(onNewGroup);
      ret->m_onNewGroupXCallback = std::move(onNewGroupX);
      ret->m_onNewGroupYCallback = std::move(onNewGroupY);
      ret->autorelease();
      return ret;
    }
    delete ret;
    return nullptr;
  }
};

class $modify(MyEditorUI, EditorUI) {
  struct Fields {
    CCMenuItemSpriteExtra *m_newGidButton = nullptr;
  };

  bool init(LevelEditorLayer *levelEditorLayer) {
    if (!EditorUI::init(levelEditorLayer))
      return false;

    if (auto menu = getChildByID("editor-buttons-menu")) {
      CCSprite *sprite = CCSprite::create("next-free-gid.png"_spr);

      m_fields->m_newGidButton = CCMenuItemSpriteExtra::create(
          sprite, this, menu_selector(MyEditorUI::onNewGid));
      m_fields->m_newGidButton->setContentSize({40, 40});
      m_fields->m_newGidButton->setID("new-gid-button");

      enableButton(m_fields->m_newGidButton, false, false);
      menu->addChild(m_fields->m_newGidButton);
      menu->updateLayout();
    }

    return true;
  }

  void showUI(bool isActive) {
    EditorUI::showUI(isActive);

    if (m_fields->m_newGidButton) {
      m_fields->m_newGidButton->setVisible(isActive);
    }
  }

  void onNewGid(CCObject *sender) { // On button press, this function checks
                                    // whether one or more obj is selected
    if (m_fields->m_newGidButton) {
      GameObject *object = nullptr;
      int selectionCount = m_selectedObjects ? m_selectedObjects->count() : 0;

      if (selectionCount > 0) { // If multiple objects are selected
        log::info("Multiple objects selected");
        object = m_selectedObject;

        auto popup = multiGIDPopup::create(
            [this]() {
              int newGroupID =
                  m_editorLayer->getNextFreeGroupID(m_selectedObjects);
              for (int i = 0; i < m_selectedObjects->count(); ++i) {
                auto obj = static_cast<GameObject *>(
                    m_selectedObjects->objectAtIndex(i));
                if (obj) {
                  obj->addToGroup(newGroupID);
                }
              }
            },                                          // New Group X callback
            [this]() { this->assignNewGroups(false); }, // New Group X callback
            [this]() { this->assignNewGroups(true); }   // New Group Y callback
        );
        popup->show();

      } else if (m_selectedObject) { // If one object is selected
        log::info("Only one object selected");
        object = m_selectedObject;
        assignNewGroups(object);
      } else {
        log::info("No objects selected"); // No objects selected
      }
    }
  }

  // void addToGroup() {
  //   if (!m_selectedObjects || m_selectedObjects->count() == 0) {
  //     log::info("No objects selected.");
  //     return;
  //   }

  //   int newGroupID = m_editorLayer->getNextFreeGroupID(m_selectedObjects);

  //   for (int i = 0; i < m_selectedObjects->count(); ++i) {
  //     auto obj = static_cast<GameObject
  //     *>(m_selectedObjects->objectAtIndex(i)); if (obj) {
  //       obj->addToGroup(newGroupID);
  //     }
  //   }

  //   log::info("Assigned group ID {} to all selected objects.", newGroupID);
  // }

  void updateButtons() { // Enables/Disables the button if objects are
                         // selected/not selected respectively
    EditorUI::updateButtons();

    if (m_fields->m_newGidButton) {
      if (m_selectedObject || m_selectedObjects->count() > 0) {
        enableButton(m_fields->m_newGidButton, true, false);
      } else {
        enableButton(m_fields->m_newGidButton, false, false);
      }
    }
  }

  // Credits to HJfod for enableButton & enableToggle:
  void enableButton(CCMenuItemSpriteExtra *btn, bool enabled, bool visualOnly) {
    btn->setEnabled(enabled || visualOnly);
    if (auto spr = typeinfo_cast<CCRGBAProtocol *>(btn->getNormalImage())) {
      spr->setCascadeColorEnabled(true);
      spr->setCascadeOpacityEnabled(true);
      spr->setColor(enabled ? ccWHITE : ccGRAY);
      spr->setOpacity(enabled ? 255 : 200);
    }
  }

  void enableToggle(CCMenuItemToggler *toggle, bool enabled, bool visualOnly) {
    toggle->setEnabled(enabled || visualOnly);
    enableButton(toggle->m_onButton, enabled, visualOnly);
    enableButton(toggle->m_offButton, enabled, visualOnly);
  }
};

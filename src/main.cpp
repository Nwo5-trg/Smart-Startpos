#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/StartPosObject.hpp>
#include <Geode/binding/EditorPauseLayer.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>

using namespace geode::prelude;

auto mod = Mod::get();

class $modify(LevelEditor, LevelEditorLayer) {
    GameObject* createObject(int p0, CCPoint p1, bool p2) {
        auto obj = LevelEditorLayer::createObject(p0, p1 ,p2);
        if (mod->getSettingValue<std::string>("mode") == "On Place") {
            if (auto startPos = dynamic_cast<StartPosObject*>(obj)) setStartPos(this->m_objects, startPos);
        }
        return obj;
    }

    void onPlaytest() {
        if (mod->getSettingValue<std::string>("mode") == "On Playtest") {
            auto objs = this->m_objects;
            for (int i = 0; i < objs->count(); i++) {
                if (auto startPos = dynamic_cast<StartPosObject*>(objs->objectAtIndex(i))) setStartPos(objs, startPos);
            }
        }
        LevelEditorLayer::onPlaytest();
    }

    void setStartPos(CCArray* objs, StartPosObject* startPos) {
        if (mod->getSettingValue<bool>("speed-toggle")) setSpeed(objs, startPos);
        if (mod->getSettingValue<bool>("gamemode-toggle")) setGamemode(objs, startPos);
        if (mod->getSettingValue<bool>("gravity-toggle")) setFlip(objs, startPos);
        if (mod->getSettingValue<bool>("mirror-toggle")) setMirror(objs, startPos);
        if (mod->getSettingValue<bool>("mini-toggle")) setMini(objs, startPos);
        if (mod->getSettingValue<bool>("dual-toggle")) setDual(objs, startPos);
    }
    
    void setSpeed(CCArray* objs, StartPosObject* startPos) {
        std::unordered_map<int, Speed> map = {
            {200, Speed::Slow}, {201, Speed::Normal}, {202, Speed::Fast}, 
            {203, Speed::Faster}, {1334, Speed::Fastest}
        };
        if (auto obj = getClosestObjectOfType({200, 201, 202, 203, 1334}, startPos, objs)) {
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) startPos->m_startSettings->m_startSpeed = entry->second;
        }
    }

    void setFlip(CCArray* objs, StartPosObject* startPos) { // to do: have more advanced logic here with mirror portals
        std::unordered_map<int, bool> map = {
            {10, false}, {11, true}
        };
        if (auto obj = getClosestObjectOfType({10, 11}, startPos, objs)) {
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) {
                bool flip = entry->second;
                if (!(getObjectsInbetweenOfType({84, 1022, 1751, 2926}, obj, startPos, objs) % 2 == 0)) flip = flip == true ? false : true;
                startPos->m_startSettings->m_isFlipped = flip;
            }
        }
    }

    void setMirror(CCArray* objs, StartPosObject* startPos) {
        std::unordered_map<int, bool> map = {
            {45, true}, {46, false}
        };
        if (auto obj = getClosestObjectOfType({45, 46}, startPos, objs)) {
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) startPos->m_startSettings->m_mirrorMode = entry->second;
        }
    }

    void setMini(CCArray* objs, StartPosObject* startPos) {
        std::unordered_map<int, bool> map = {
            {99, false}, {101, true}
        };
        if (auto obj = getClosestObjectOfType({99, 101}, startPos, objs)) {
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) startPos->m_startSettings->m_startMini = entry->second;
        }
    }

    void setDual(CCArray* objs, StartPosObject* startPos) {
        std::unordered_map<int, bool> map = {
            {286, true}, {287, false}
        };
        if (auto obj = getClosestObjectOfType({286, 287}, startPos, objs)) {
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) {
                startPos->m_startSettings->m_startDual = entry->second;
                toggleFreemodeOn(static_cast<EffectGameObject*>(obj), startPos);
            }
        }
    }

    void toggleFreemodeOn(EffectGameObject* obj, StartPosObject* startPos) {
        if (Loader::get()->isModLoaded("blueblock6.cleanstartpos") && obj->m_cameraIsFreeMode) { // add freemode with clean startpos
            auto editUI = EditorUI::get();
            editUI->selectObject(startPos, true);
            editUI->m_editObjectBtn->activate();
            if (auto settingsLayer = CCDirector::sharedDirector()->getRunningScene()->getChildByType<LevelSettingsLayer>(0)) {
                auto toggler = static_cast<CCMenuItemToggler*>(settingsLayer->getChildByIDRecursive("blueblock6.cleanstartpos/free-mode-toggle"));
                if (!toggler->isToggled()) toggler->activate();
                settingsLayer->getChildByType<CCLayer>(0)->getChildByType<CCMenu>(0)->getChildByType<CCMenuItemSpriteExtra>(0)->activate();
            }
        }
    }

    void setGamemode(CCArray* objs, StartPosObject* startPos) {
        std::unordered_map<int, int> map = {
            {12, 0}, {13, 1}, {47, 2}, {111, 3}, 
            {660, 4}, {745, 5}, {1331, 6}, {1933, 7},
        };
        if (auto obj = getClosestObjectOfType({12, 13, 47, 111, 660, 745, 1331, 1933}, startPos, objs)) {
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) {
                startPos->m_startSettings->m_startMode = entry->second;
                toggleFreemodeOn(static_cast<EffectGameObject*>(obj), startPos);
            }
        }
    }
    
    GameObject* getClosestObjectOfType(std::unordered_set<int> ids, GameObject* objectFrom, CCArray* objs) {
        auto distance = INT_MAX;
        GameObject* closestObject = nullptr;
    
        for (int i = 0; i < objs->count(); i++) {
            auto obj = static_cast<GameObject*>(objs->objectAtIndex(i));       
            if (obj->getPosition().x < objectFrom->getPosition().x && obj->getPosition().x - objectFrom->getPosition().x < 0 
            && std::abs(obj->getPosition().x - objectFrom->getPosition().x) < distance && ids.contains(obj->m_objectID) && !obj->m_isNoTouch) {
                closestObject = obj;
                distance = std::abs(obj->getPosition().x - objectFrom->getPosition().x);
            }
        }
        return closestObject;
    }

    int getObjectsInbetweenOfType(std::unordered_set<int> ids, GameObject* objectFrom, GameObject* objectTo, CCArray* objs) {
        int objects = 0;
        for (int i = 0; i < objs->count(); i++) {
            auto obj = static_cast<GameObject*>(objs->objectAtIndex(i));  
            if (obj->getPosition().x < objectTo->getPosition().x && obj->getPosition().x > objectFrom->getPosition().x && ids.contains(obj->m_objectID) && !obj->m_isNoTouch) objects++;
        }
        log::error("{}", objects);
        return objects;
    }
};
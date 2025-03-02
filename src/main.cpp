#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/StartPosObject.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/EditorPauseLayer.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <unordered_map>

using namespace geode::prelude;

CCArray* getAllObjectsFromSelectAll() {
    auto editUI = EditorUI::get();
    auto levelEditor = LevelEditorLayer::get();
    auto pauseLayer = EditorPauseLayer::create(levelEditor);
    auto originalObjs = editUI->getSelectedObjects();
    pauseLayer->getChildByType<CCMenu>(2)->getChildByType<CCMenuItemSpriteExtra>(4)->activate();
    pauseLayer->removeMeAndCleanup();
    auto objs = editUI->getSelectedObjects();
    editUI->deselectAll();
    editUI->selectObjects(originalObjs, true);
    editUI->updateObjectInfoLabel();
    editUI->updateButtons();
    editUI->updateDeleteButtons();
    return objs;
}

GameObject* getClosestObjectOfType(std::unordered_set<int> ids, GameObject* objectFrom, CCArray* objs) {
    auto distance = INT_MAX;
    GameObject* closestObject;

    for (int i = 0; i < objs->count(); i++) {
        auto obj = static_cast<GameObject*>(objs->objectAtIndex(i));       
        if (obj->getPosition().x < objectFrom->getPosition().x && obj->getPosition().x - objectFrom->getPosition().x < 0 
        && std::abs(obj->getPosition().x - objectFrom->getPosition().x) < distance && ids.contains(obj->m_objectID)) {
            closestObject = obj;
            distance = std::abs(obj->getPosition().x - objectFrom->getPosition().x);
        }
    }
    return closestObject;
}

class $modify(LevelEditor, LevelEditorLayer) {
    GameObject* createObject(int p0, CCPoint p1, bool p2) {
        auto obj = LevelEditorLayer::createObject(p0, p1 ,p2);
        
        if (auto startPos = dynamic_cast<StartPosObject*>(obj)) {
            auto objs = getAllObjectsFromSelectAll();
            setSpeed(objs, startPos);
            setGamemode(objs, startPos);
            setFlip(objs, startPos);
            setMirror(objs, startPos);
            setMini(objs, startPos);
            setDual(objs, startPos);
        }
        return obj;
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
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) startPos->m_startSettings->m_isFlipped = entry->second;
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
            if (auto entry = map.find(obj->m_objectID); entry != map.end()) startPos->m_startSettings->m_startDual = entry->second;
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
                if (Loader::get()->isModLoaded("blueblock6.cleanstartpos") && static_cast<EffectGameObject*>(obj)->m_cameraIsFreeMode) { // add freemode with clean startpos
                    auto editUI = EditorUI::get();
                    editUI->selectObject(startPos, true);
                    editUI->m_editObjectBtn->activate();
                    if (auto settingsLayer = CCDirector::sharedDirector()->getRunningScene()->getChildByType<LevelSettingsLayer>(0)) {
                        static_cast<CCMenuItemToggler*>(settingsLayer->getChildByIDRecursive("blueblock6.cleanstartpos/free-mode-toggle"))->toggleWithCallback(true);;
                        settingsLayer->getChildByType<CCLayer>(0)->getChildByType<CCMenu>(0)->getChildByType<CCMenuItemSpriteExtra>(0)->activate();
                    }
                }
            }
        }
    }
};

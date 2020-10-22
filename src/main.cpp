#include "include/main.hpp"

static ModInfo modInfo;

GlobalNamespace::StandardLevelFailedController* standardLevelFailedController;

static Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

bool alreadyFailed = false;

std::string menuSceneName = "MenuViewControllers";
std::string gameSceneName = "GameCore";
MAKE_HOOK_OFFSETLESS(ActiveSceneChanged, void, UnityEngine::SceneManagement::Scene previousActiveScene, UnityEngine::SceneManagement::Scene newActiveScene)
{
    ActiveSceneChanged(previousActiveScene, newActiveScene);
    Il2CppString* sceneName = UnityEngine::SceneManagement::Scene::GetNameInternal(newActiveScene.m_Handle);
    std::string cppSceneStr = to_utf8(csstrtostr(sceneName));

    if(cppSceneStr == gameSceneName)
    {
        alreadyFailed = false;

        Array<GlobalNamespace::StandardLevelFailedController*>* standardLevelArray = UnityEngine::Resources::FindObjectsOfTypeAll<GlobalNamespace::StandardLevelFailedController*>();
        if(standardLevelArray)
        {
            //.First()
            standardLevelFailedController = standardLevelArray->values[0];
        }

    }

    if(cppSceneStr == menuSceneName)
    {
        
    }
}

float FailThresholdValue = 80.0f; 

MAKE_HOOK_OFFSETLESS(UpdateRelativeScoreAndImmediateRank, void, GlobalNamespace::RelativeScoreAndImmediateRankCounter* self, int score, int modifiedScore, int maxPossibleScore, int maxPossibleModifiedScore)
{
    UpdateRelativeScoreAndImmediateRank(self, score, modifiedScore, maxPossibleScore, maxPossibleModifiedScore);
    float currentAcc = self->relativeScore * 100.0f;

    if(!alreadyFailed && currentAcc < FailThresholdValue)
    {
        alreadyFailed = true;
        standardLevelFailedController->HandleLevelFailed();
    }
}

extern "C" void setup(ModInfo &info)
{
    info.id = "qInstaFailAccuracy";
    info.version = "0.1.0";
    modInfo = info;
    getConfig();
    getLogger().info("Completed setup!");
    getLogger().info("Modloader name: %s", Modloader::getInfo().name.c_str());
}  

extern "C" void load()
{ 
    INSTALL_HOOK_OFFSETLESS(UpdateRelativeScoreAndImmediateRank, il2cpp_utils::FindMethodUnsafe("", "RelativeScoreAndImmediateRankCounter", "UpdateRelativeScoreAndImmediateRank", 4));
    INSTALL_HOOK_OFFSETLESS(ActiveSceneChanged, il2cpp_utils::FindMethodUnsafe("UnityEngine.SceneManagement", "SceneManager", "Internal_ActiveSceneChanged", 2));
}
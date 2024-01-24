///------------------------------------------------------------------------------------------------
///  CardSelectionRewardSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/MeshResource.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/SceneObjectUtils.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/CardTooltipController.h>
#include <game/GuiObjectManager.h>
#include <game/events/EventSystem.h>
#include <game/DataRepository.h>
#include <game/scenelogicmanagers/CardPackRewardSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId OPEN_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("open_button");

static const std::string CARD_PACK_REWARD_SCENE_OBJECT_NAME = "card_pack_reward";
static const std::string CARD_PACK_REWARD_MESH_FILE_NAME = "card_pack_dynamic.obj";
//static const std::string GOLDEN_CARD_PACK_SHADER_FILE_NAME = "card_pack_golden.vs";
//static const std::string CARD_PACK_REWARD_TEXTURE_FILE_NAME = "card_pack_golden.png";
static const std::string NORMAL_CARD_PACK_SHADER_FILE_NAME = "basic.vs";
static const std::string NORMAL_CARD_PACK_TEXTURE_FILE_NAME = "card_pack_normal.png";

static const float SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS = 0.5f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.1f;

static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 OPEN_BUTTON_POSITION = {-0.085f, -0.18f, 23.1f};
static const glm::vec3 PACK_VERTEX_GRAVITY = {0.0f, -0.00008f, 0.0f};
static const glm::vec3 CARD_PACK_INIT_POSITION = {0.0f, 0.0f, 23.2f};
static const glm::vec3 CARD_PACK_INIT_SCALE = {1/60.0f, 1/60.0f, 1/60.0f};
static const glm::vec3 CARD_PACK_TARGET_SCALE = {1.25f/60.0f, 1.25f/60.0f, 1.25f/60.0f};

static const float PACK_EXPLOSION_NOISE_MAG = 0.006f;
static const float PACK_EXPLOSION_VELOCITY_MAG = 0.06f;
static const float PACK_EXPLOSION_ALPHA_REDUCTION_SPEED = 0.001f;
static const float PACK_SHAKE_STEP_DURATION = 0.01f;
static const float PACK_SHAKE_POSITION_NOISE_MAGNITUDE = 0.02f;
static const float PACK_SHAKE_SCALE_ANIMATION_DURATION_SECS = 2.0f;
static const float PACK_EXPLOSION_ALPHA_REDUCTION_ANIMATION_DURATION_SECS = 1.0f;
static constexpr int PACK_MAX_SHAKE_STEPS = 100;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::CARD_PACK_REWARD_SCENE_NAME
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    game_constants::OVERLAY_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& CardPackRewardSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

CardPackRewardSceneLogicManager::CardPackRewardSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

CardPackRewardSceneLogicManager::~CardPackRewardSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mSceneState = SceneState::PENDING_PACK_OPENING;
    mCardPackShakeStepsRemaining = PACK_MAX_SHAKE_STEPS;
    
    auto cardPackReward = scene->CreateSceneObject(strutils::StringId(CARD_PACK_REWARD_SCENE_OBJECT_NAME));
    cardPackReward->mPosition = CARD_PACK_INIT_POSITION;
    cardPackReward->mScale = CARD_PACK_INIT_SCALE;
    
    CoreSystemsEngine::GetInstance().GetResourceLoadingService().UnloadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + CARD_PACK_REWARD_MESH_FILE_NAME);
    cardPackReward->mMeshResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + CARD_PACK_REWARD_MESH_FILE_NAME);
    cardPackReward->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + NORMAL_CARD_PACK_TEXTURE_FILE_NAME);
    cardPackReward->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + NORMAL_CARD_PACK_SHADER_FILE_NAME);
    cardPackReward->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    mOpenButton = std::make_unique<AnimatedButton>
    (
        OPEN_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Open Pack",
        OPEN_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(cardPackReward->mRotation.y, 0.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS/2, animation_flags::NONE), [=](){});
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mOpenButton->GetSceneObject(), 0.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE), [=](){ mOpenButton->GetSceneObject()->mInvisible = true; });
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(cardPackReward->mScale.x, CARD_PACK_TARGET_SCALE.x, PACK_SHAKE_SCALE_ANIMATION_DURATION_SECS, animation_flags::NONE), [=](){});
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(cardPackReward->mScale.y, CARD_PACK_TARGET_SCALE.y, PACK_SHAKE_SCALE_ANIMATION_DURATION_SECS, animation_flags::NONE), [=](){});
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>
            (
                cardPackReward,
                glm::vec3(CARD_PACK_INIT_POSITION.x + math::RandomFloat(-PACK_SHAKE_POSITION_NOISE_MAGNITUDE, PACK_SHAKE_POSITION_NOISE_MAGNITUDE), CARD_PACK_INIT_POSITION.y + math::RandomFloat(-PACK_SHAKE_POSITION_NOISE_MAGNITUDE, PACK_SHAKE_POSITION_NOISE_MAGNITUDE), CARD_PACK_INIT_POSITION.z),
                CARD_PACK_INIT_SCALE,
                PACK_SHAKE_STEP_DURATION,
                animation_flags::IGNORE_SCALE
            ), [=](){ CardPackShakeStep(scene); });
            
            PreparePackVertexVelocities(scene);
            mSceneState = SceneState::PACK_SHAKING;
        },
        *scene
    );
    
    size_t sceneObjectIndex = 0;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        sceneObject->mInvisible = false;
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=](){});
    }
    RegisterForEvents();
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    static float time = 0.0f;
    time += dtMillis * 0.001f;
    
    auto cardPackReward = scene->FindSceneObject(strutils::StringId(CARD_PACK_REWARD_SCENE_OBJECT_NAME));
    cardPackReward->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
    
    switch (mSceneState)
    {
        case SceneState::PENDING_PACK_OPENING:
        {
            cardPackReward->mRotation.y = math::Sinf(time);
            mOpenButton->Update(dtMillis);
        } break;
            
        case SceneState::PACK_SHAKING:
        {
        } break;
            
        case SceneState::PACK_EXPLODING:
        {
            UpdatePackVertices(dtMillis, scene);
            
            cardPackReward->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Max(0.0f, cardPackReward->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] - PACK_EXPLOSION_ALPHA_REDUCTION_SPEED * dtMillis);
            
            if (CoreSystemsEngine::GetInstance().GetInputStateManager().VButtonTapped(input::Button::MIDDLE_BUTTON))
            {
                mOpenButton->GetSceneObject()->mInvisible = false;
                mOpenButton->GetSceneObject()->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                
                auto cardPackReward = scene->FindSceneObject(strutils::StringId(CARD_PACK_REWARD_SCENE_OBJECT_NAME));
                cardPackReward->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                
                mCardPackVertexVelocities.clear();
                CoreSystemsEngine::GetInstance().GetResourceLoadingService().UnloadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + CARD_PACK_REWARD_MESH_FILE_NAME);
                cardPackReward->mMeshResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + CARD_PACK_REWARD_MESH_FILE_NAME);
                cardPackReward->mScale = CARD_PACK_INIT_SCALE;
                mCardPackShakeStepsRemaining = PACK_MAX_SHAKE_STEPS;
                mSceneState = SceneState::PENDING_PACK_OPENING;
            }
        } break;
    }
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            sceneObject->mInvisible = true;
        });
    }
    
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> CardPackRewardSceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &CardPackRewardSceneLogicManager::OnWindowResize);
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::CARD_PACK_REWARD_SCENE_NAME)->RecalculatePositionOfEdgeSnappingSceneObjects();
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::PreparePackVertexVelocities(std::shared_ptr<scene::Scene> scene)
{
    auto cardPackReward = scene->FindSceneObject(strutils::StringId(CARD_PACK_REWARD_SCENE_OBJECT_NAME));
    auto& cardPackMesh = CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetResource<resources::MeshResource>(cardPackReward->mMeshResourceId);
    
    cardPackMesh.ApplyDirectTransformToData([=](resources::MeshResource::MeshData& meshData)
    {
        mCardPackVertexVelocities.resize(meshData.mVertices.size());
        
        for (int i = 0; i < meshData.mVertices.size(); ++i)
        {
            auto randomVelocityOffset = glm::vec3(math::RandomFloat(-PACK_EXPLOSION_NOISE_MAG, PACK_EXPLOSION_NOISE_MAG), math::RandomFloat(-PACK_EXPLOSION_NOISE_MAG, PACK_EXPLOSION_NOISE_MAG), 0.0f);
            if (math::Abs(meshData.mNormals[i].z) > 0.8)
            {
                
                mCardPackVertexVelocities[i] = glm::normalize(meshData.mVertices[i]) * PACK_EXPLOSION_VELOCITY_MAG + randomVelocityOffset;
                mCardPackVertexVelocities[i + 1] = glm::normalize(meshData.mVertices[i]) * PACK_EXPLOSION_VELOCITY_MAG + randomVelocityOffset;
                mCardPackVertexVelocities[i + 2] = glm::normalize(meshData.mVertices[i]) * PACK_EXPLOSION_VELOCITY_MAG + randomVelocityOffset;

                i += 2;
            }
            else
            {
                mCardPackVertexVelocities[i] += glm::normalize(meshData.mNormals[i]) * PACK_EXPLOSION_VELOCITY_MAG + randomVelocityOffset;
            }
        }
    });
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::UpdatePackVertices(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    auto cardPackReward = scene->FindSceneObject(strutils::StringId(CARD_PACK_REWARD_SCENE_OBJECT_NAME));
    auto& cardPackMesh = CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetResource<resources::MeshResource>(cardPackReward->mMeshResourceId);
    
    cardPackMesh.ApplyDirectTransformToData([=](resources::MeshResource::MeshData& meshData)
    {
        for (int i = 0; i < meshData.mVertices.size(); ++i)
        {
            float oldZ = meshData.mVertices[i].z;
            mCardPackVertexVelocities[i] += PACK_VERTEX_GRAVITY * dtMillis;
            meshData.mVertices[i] += mCardPackVertexVelocities[i] * dtMillis;
            meshData.mVertices[i].z = oldZ;
        }
    });
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::CardPackShakeStep(std::shared_ptr<scene::Scene> scene)
{
    auto cardPackReward = scene->FindSceneObject(strutils::StringId(CARD_PACK_REWARD_SCENE_OBJECT_NAME));
    
    if (mCardPackShakeStepsRemaining-- == 0)
    {
        mSceneState = SceneState::PACK_EXPLODING;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cardPackReward, 0.0f, PACK_EXPLOSION_ALPHA_REDUCTION_ANIMATION_DURATION_SECS, animation_flags::NONE), [=]()
        {
            cardPackReward->mInvisible = true;
            CreateCardRewards(scene);
        });
    }
    else
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>
        (
            cardPackReward,
            glm::vec3(CARD_PACK_INIT_POSITION.x + math::RandomFloat(-PACK_SHAKE_POSITION_NOISE_MAGNITUDE, PACK_SHAKE_POSITION_NOISE_MAGNITUDE), CARD_PACK_INIT_POSITION.y + math::RandomFloat(-PACK_SHAKE_POSITION_NOISE_MAGNITUDE, PACK_SHAKE_POSITION_NOISE_MAGNITUDE), CARD_PACK_INIT_POSITION.z),
            cardPackReward->mScale,
            PACK_SHAKE_STEP_DURATION,
            animation_flags::IGNORE_SCALE
        ), [=](){ CardPackShakeStep(scene); });
    }
}

///------------------------------------------------------------------------------------------------

void CardPackRewardSceneLogicManager::CreateCardRewards(std::shared_ptr<scene::Scene>)
{
    auto cardRewardPool = CardDataRepository::GetInstance().GetCardPackLockedCardRewardsPool();
    cardRewardPool.clear();
}

///------------------------------------------------------------------------------------------------

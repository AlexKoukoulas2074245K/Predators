///------------------------------------------------------------------------------------------------
///  Events.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/11/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Events_h
#define Events_h

///------------------------------------------------------------------------------------------------

#include <game/CardEffectComponents.h>
#include <game/Cards.h>

///------------------------------------------------------------------------------------------------

namespace events
{

///------------------------------------------------------------------------------------------------

class ApplicationMovedToBackgroundEvent final
{
};

///------------------------------------------------------------------------------------------------

class WindowResizeEvent final
{
};

///------------------------------------------------------------------------------------------------

class LocalPlayerTurnStarted final
{
};

///------------------------------------------------------------------------------------------------

class EndOfTurnCardDestructionEvent final
{
public:
    EndOfTurnCardDestructionEvent(const std::vector<std::string> cardIndices, const bool isBoardCard, const bool forRemotePlayer)
        : mCardIndices(cardIndices)
        , mIsBoardCard(isBoardCard)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const std::vector<std::string> mCardIndices;
    const bool mIsBoardCard;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class ImmediateCardDestructionWithRepositionEvent final
{
public:
    ImmediateCardDestructionWithRepositionEvent(const int cardIndex, const bool isBoardCard, const bool forRemotePlayer)
        : mCardIndex(cardIndex)
        , mIsBoardCard(isBoardCard)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const int mCardIndex;
    const bool mIsBoardCard;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class CardCreationEvent final
{
public:
    CardCreationEvent(std::shared_ptr<CardSoWrapper> cardSoWrapper, const bool forRemotePlayer)
        : mCardSoWrapper(cardSoWrapper)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const std::shared_ptr<CardSoWrapper> mCardSoWrapper;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class CardBuffedDebuffedEvent final
{
public:
    CardBuffedDebuffedEvent(const int cardIndex, const bool boardCard, const bool forRemotePlayer)
        : mCardIndex(cardIndex)
        , mBoardCard(boardCard)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const int mCardIndex;
    const bool mBoardCard;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class ForceSendCardBackToPositionEvent final
{
public:
    ForceSendCardBackToPositionEvent(const int cardIndex, const bool boardCard, const bool forRemotePlayer)
        : mCardIdex(cardIndex)
        , mBoardCard(boardCard)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const int mCardIdex;
    const bool mBoardCard;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class BoardSideCardEffectTriggeredEvent final
{
public:
    BoardSideCardEffectTriggeredEvent(const bool forRemotePlayer, const effects::EffectBoardModifierMask effectBoardModifierMask)
        : mForRemotePlayer(forRemotePlayer)
        , mEffectBoardModifierMask(effectBoardModifierMask)
    {
    }
    
    const bool mForRemotePlayer;
    const effects::EffectBoardModifierMask mEffectBoardModifierMask;
};

///------------------------------------------------------------------------------------------------

class BoardSideCardEffectEndedEvent final
{
public:
    BoardSideCardEffectEndedEvent(const bool forRemotePlayer, const bool massClear, const effects::EffectBoardModifierMask effectBoardModifierMask)
        : mForRemotePlayer(forRemotePlayer)
        , mMassClear(massClear)
        , mEffectBoardModifierMask(effectBoardModifierMask)
    {
    }
    
    const bool mForRemotePlayer;
    const bool mMassClear;
    const effects::EffectBoardModifierMask mEffectBoardModifierMask;
};

///------------------------------------------------------------------------------------------------

class HeldCardSwapEvent final
{
public:
    HeldCardSwapEvent(const std::shared_ptr<CardSoWrapper> cardSoWrapper, const int cardIndex, const bool forRemotePlayer)
        : mCardSoWrapper(cardSoWrapper)
        , mCardIndex(cardIndex)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const std::shared_ptr<CardSoWrapper> mCardSoWrapper;
    const int mCardIndex;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class NewBoardCardCreatedEvent final
{
public:
    NewBoardCardCreatedEvent(const std::shared_ptr<CardSoWrapper> cardSoWrapper, const int cardIndex, const bool forRemotePlayer)
        : mCardSoWrapper(cardSoWrapper)
        , mCardIndex(cardIndex)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const std::shared_ptr<CardSoWrapper> mCardSoWrapper;
    const int mCardIndex;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class LastCardPlayedFinalizedEvent final
{
public:
    LastCardPlayedFinalizedEvent(const int cardIndex)
        : mCardIndex(cardIndex)
    {
    }
    
    const int mCardIndex;
};

///------------------------------------------------------------------------------------------------

class WeightChangeAnimationTriggerEvent final
{
public:
    WeightChangeAnimationTriggerEvent(const bool forRemotePlayer)
        : mForRemotePlayer(forRemotePlayer)
    {
        
    }
    
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class HealthChangeAnimationTriggerEvent final
{
public:
    HealthChangeAnimationTriggerEvent(const bool forRemotePlayer)
        : mForRemotePlayer(forRemotePlayer)
    {
        
    }
    
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class PoisonStackChangeChangeAnimationTriggerEvent final
{
public:
    PoisonStackChangeChangeAnimationTriggerEvent(const bool forRemotePlayer, const int newPoisonStackValue)
        : mForRemotePlayer(forRemotePlayer)
        , mNewPoisonStackValue(newPoisonStackValue)
    {
        
    }
    
    const bool mForRemotePlayer;
    const int mNewPoisonStackValue;
};

///------------------------------------------------------------------------------------------------

class CardHistoryEntryAdditionEvent final
{
public:
    CardHistoryEntryAdditionEvent(const bool forRemotePlayer, const bool isTurnCounter, const int cardIndex, const std::string& entryTypeTextureFileName)
        : mForRemotePlayer(forRemotePlayer)
        , mIsTurnCounter(isTurnCounter)
        , mCardIndex(cardIndex)
        , mEntryTypeTextureFileName(entryTypeTextureFileName)
    {
    }
    
    const bool mForRemotePlayer;
    const bool mIsTurnCounter;
    const int mCardIndex;
    const std::string mEntryTypeTextureFileName;
};

///------------------------------------------------------------------------------------------------

class SceneChangeEvent final
{
public:
    SceneChangeEvent(const strutils::StringId& newSceneName, const bool isModal, const bool useLoadingScene, const float targetDurationSecs = 0.0f, const float maxTransitionDarkeningAlpha = 0.0f)
        : mNewSceneName(newSceneName)
        , mIsModal(isModal)
        , mUseLoadingScene(useLoadingScene)
        , mTargetDurationSecs(targetDurationSecs)
        , mMaxTransitionDarkeningAlpha(maxTransitionDarkeningAlpha)
    {
        
    }
    
    const strutils::StringId mNewSceneName;
    const bool mIsModal;
    const bool mUseLoadingScene;
    const float mTargetDurationSecs;
    const float mMaxTransitionDarkeningAlpha;
};

///------------------------------------------------------------------------------------------------

class PopSceneModalEvent final
{
public:
    PopSceneModalEvent(const float targetDurationSecs = 0.0f, const float maxTransitionDarkeningAlpha = 0.0f)
        : mTargetDurationSecs(targetDurationSecs)
        , mMaxTransitionDarkeningAlpha(maxTransitionDarkeningAlpha)
    {
        
    }
    
    const float mTargetDurationSecs;
    const float mMaxTransitionDarkeningAlpha;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Events_h */

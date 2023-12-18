///------------------------------------------------------------------------------------------------
///  ProgressionDataRepository.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ProgressionDataRepository_h
#define ProgressionDataRepository_h

///------------------------------------------------------------------------------------------------

#include <vector>

///------------------------------------------------------------------------------------------------

enum class BattleControlType
{
    REPLAY,
    AI_TOP_BOT,
    AI_TOP_ONLY
};

///------------------------------------------------------------------------------------------------

class ProgressionDataRepository final
{
public:
    static ProgressionDataRepository& GetInstance();
    ~ProgressionDataRepository() = default;
    
    ProgressionDataRepository(const ProgressionDataRepository&) = delete;
    ProgressionDataRepository(ProgressionDataRepository&&) = delete;
    const ProgressionDataRepository& operator = (const ProgressionDataRepository&) = delete;
    ProgressionDataRepository& operator = (ProgressionDataRepository&&) = delete;
    
    BattleControlType GetNextBattleControlType() const;
    void SetNextBattleControlType(const BattleControlType nextBattleControlType);
    
    const std::vector<int>& GetNextTopPlayerDeck() const;
    void SetNextTopPlayerDeck(const std::vector<int>& deck);
    
    const std::vector<int>& GetNextBotPlayerDeck() const;
    void SetNextBotPlayerDeck(const std::vector<int>& deck);
    
    int GetCurrencyCoins() const;
    void SetCurrencyCoins(const int currencyCoins);
    
private:
    ProgressionDataRepository() = default;
    
private:
    BattleControlType mNextBattleControlType;
    std::vector<int> mNextTopPlayerDeck;
    std::vector<int> mNextBotPlayerDeck;
    int mCurrencyCoins = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* ProgressionDataRepository_h */

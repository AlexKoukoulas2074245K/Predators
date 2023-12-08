///------------------------------------------------------------------------------------------------
///  ProgressionDataRepository.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ProgressionDataRepository_h
#define ProgressionDataRepository_h

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
    
private:
    ProgressionDataRepository() = default;
    
private:
    BattleControlType mNextBattleControlType;
};

///------------------------------------------------------------------------------------------------

#endif /* ProgressionDataRepository_h */

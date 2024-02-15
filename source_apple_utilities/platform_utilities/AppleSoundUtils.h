///------------------------------------------------------------------------------------------------
///  AppleSoundUtils.h
///  Predators
///
///  Created by Alex Koukoulas on 14/02/2024.
///-----------------------------------------------------------------------------------------------

#ifndef AppleSoundUtils_h
#define AppleSoundUtils_h

///-----------------------------------------------------------------------------------------------

#include <string>

///-----------------------------------------------------------------------------------------------

namespace sound_utils
{

///-----------------------------------------------------------------------------------------------

void Vibrate();
void PreloadSfx(const std::string& sfxResPath);
void PlaySound(const std::string& soundResPath, const bool loopedSfx = false);
void InitAudio();
void ResumeAudio();
void PauseMusicOnly();
void PauseSfxOnly();
void PauseAudio();
void UpdateAudio(const float dtMillis);
void SetAudioEnabled(const bool audioEnabled);

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------

#endif /* AppleSoundUtils_h */

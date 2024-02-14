///------------------------------------------------------------------------------------------------
///  AppleSoundUtils.cpp
///  Predators
///
///  Created by Alex Koukoulas on 14/02/2024.
///-----------------------------------------------------------------------------------------------

#include <platform_utilities/AppleSoundUtils.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/FileUtils.h>
#include <engine/utils/OSMessageBox.h>
#include <engine/utils/StringUtils.h>

#import <platform_utilities/AVAudioPlayerManager.h>
#import <AudioToolbox/AudioServices.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>


///-----------------------------------------------------------------------------------------------

namespace sound_utils
{

///-----------------------------------------------------------------------------------------------

static AVAudioPlayerManager* manager;
static std::string ROOT_SOUND_RES_PATH;

///-----------------------------------------------------------------------------------------------

void Vibrate()
{
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

///-----------------------------------------------------------------------------------------------

void PreloadSfx(const std::string& sfxResPath)
{
    if (strutils::StringEndsWith(sfxResPath, ".mp3") || strutils::StringEndsWith(sfxResPath, ".wav"))
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::WARNING, "Sound Path contains extension", "Sound res path " + sfxResPath + " includes extension and will be ignored.");
        return;
    }
    
    NSString* objectiveCresPath = [NSString stringWithCString:sfxResPath.data() encoding:[NSString defaultCStringEncoding]];
    
    if (manager != nil)
    {
        [manager preloadSfxWith:objectiveCresPath];
    }
}

///------------------------------------------------------------------------------------------------

void PlaySound(const std::string& soundResPath, const bool loopedSfx /* = false */)
{
    if (strutils::StringEndsWith(soundResPath, ".flac") || strutils::StringEndsWith(soundResPath, ".mp3"))
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::WARNING, "Sound Path contains extension", "Sound res path " + soundResPath + " includes extension and will be ignored.");
        return;
    }
    
    NSString* objectiveCresPath = [NSString stringWithCString:(ROOT_SOUND_RES_PATH + soundResPath).data() encoding:[NSString defaultCStringEncoding]];
    
    if (manager != nil)
    {
        [manager playSoundWith:objectiveCresPath isMusic:(!strutils::StringStartsWith(fileutils::GetFileName(soundResPath), "sfx_")) forceLoop:loopedSfx];
    }
}

///------------------------------------------------------------------------------------------------

void InitAudio()
{
    ROOT_SOUND_RES_PATH = "assets/music/";
    manager = [[AVAudioPlayerManager alloc] init];
}

///------------------------------------------------------------------------------------------------

void ResumeAudio()
{
    if (manager != nil)
    {
        [manager resumeAudio];
    }
}

///------------------------------------------------------------------------------------------------

void PauseMusicOnly()
{
    if (manager != nil)
    {
        [manager pauseMusic];
    }
}

///------------------------------------------------------------------------------------------------

void PauseSfxOnly()
{
    if (manager != nil)
    {
        [manager pauseSfx];
    }
}

///------------------------------------------------------------------------------------------------

void PauseAudio()
{
    if (manager != nil)
    {
        [manager pauseAudio];
    }
}

///------------------------------------------------------------------------------------------------

void UpdateAudio(const float dtMillis)
{
    if (manager != nil)
    {
        [manager updateAudioWith:dtMillis];
    }
}

///------------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------

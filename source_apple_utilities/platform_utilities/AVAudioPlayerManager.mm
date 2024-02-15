///------------------------------------------------------------------------------------------------
///  AVAudioPlayerManager.cpp
///  Predators
///
///  Created by Alex Koukoulas on 14/02/2024.
///-----------------------------------------------------------------------------------------------

#import <platform_utilities/AVAudioPlayerManager.h>

///------------------------------------------------------------------------------------------------

@implementation AVAudioPlayerManager

///------------------------------------------------------------------------------------------------

static const float ENABLED_AUDIO_MUSIC_VOLUME = 1.0f;
static const float DISABLED_AUDIO_MUSIC_VOLUME = 0.0f;
static const float ENABLED_AUDIO_SFX_VOLUME = 0.2f;
static const float DISABLED_AUDIO_SFX_VOLUME = 0.0f;

///------------------------------------------------------------------------------------------------

-(id) init
{
    self = [super init];
    _sfxPlayers = [[NSMutableDictionary alloc] init];
    _currentPlayingMusicPath = nil;
    _nextQueuedMusicPath = nil;
    _musicPlayer = nil;
    _firstAppStateCall = YES;
    _targetSfxVolume = ENABLED_AUDIO_SFX_VOLUME;
    _targetMusicVolume = ENABLED_AUDIO_MUSIC_VOLUME;
    return self;
}

///------------------------------------------------------------------------------------------------

- (void) preloadSfxWith:(NSString*) sfxResPath
{
    NSString* sandboxFilePath = [NSBundle.mainBundle pathForResource:sfxResPath ofType:@"flac"];
    
    if (sandboxFilePath != nil)
    {
        if ([_sfxPlayers objectForKey:sandboxFilePath] == nil)
        {
            NSError *error;
            AVAudioPlayer* sfxPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:sandboxFilePath] error:&error];
            
            if (error)
            {
                NSLog(@"%@",[error localizedDescription]);
            }
            
            sfxPlayer.numberOfLoops = 0;
            [sfxPlayer prepareToPlay];
            
            _sfxPlayers[sandboxFilePath] = sfxPlayer;
        }
    }
}

///------------------------------------------------------------------------------------------------

- (void) playSoundWith: (NSString*) soundResPath isMusic:(BOOL) isMusic forceLoop:(BOOL) forceLoop
{
    NSString* sandboxFilePath = [NSBundle.mainBundle pathForResource:soundResPath ofType:@"flac"];
    
    if (sandboxFilePath != nil)
    {
        if (isMusic)
        {
            if (_musicPlayer == nil)
            {
                dispatch_queue_t backgroundQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0);
                dispatch_async(backgroundQueue, ^{
                    self->_musicPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:sandboxFilePath] error:nil];
                    self->_musicPlayer.numberOfLoops = -1;
                    self->_musicPlayer.volume = 0.0f;
                    self->_currentPlayingMusicPath = sandboxFilePath;
                    self->_nextQueuedMusicPath = sandboxFilePath;
                    
                    [self->_musicPlayer prepareToPlay];
                    [self->_musicPlayer play];
                });
            }
            else if (![_currentPlayingMusicPath isEqualToString:sandboxFilePath])
            {
                _nextQueuedMusicPath = sandboxFilePath;
            }
        }
        else
        {
            AVAudioPlayer* targetSfxPlayer = [_sfxPlayers objectForKey:sandboxFilePath];
            if (targetSfxPlayer)
            {
                dispatch_queue_t backgroundQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0);
                dispatch_async(backgroundQueue, ^{
                    if (forceLoop)
                    {
                        targetSfxPlayer.numberOfLoops = 1;
                    }
                    targetSfxPlayer.currentTime = 0;
                    targetSfxPlayer.volume = self.targetSfxVolume;
                    [targetSfxPlayer play];
                });
            }
            else
            {
                NSLog(@"Can't find audio player for %@", sandboxFilePath);
            }
        }
    }
    else
    {
        NSLog(@"Can't open sound file %@", soundResPath);
    }
}

///------------------------------------------------------------------------------------------------

- (void) pauseMusic
{
    if (_musicPlayer != nil)
    {
        [_musicPlayer pause];
    }
}

///------------------------------------------------------------------------------------------------

- (void) pauseSfx
{
    for(id key in _sfxPlayers)
    {
        AVAudioPlayer* sfxPlayer = [_sfxPlayers objectForKey:key];
        [sfxPlayer pause];
    }
}

///------------------------------------------------------------------------------------------------

- (void) pauseAudio
{
    [self pauseSfx];
    [self pauseMusic];
}

///------------------------------------------------------------------------------------------------

- (void) resumeAudio
{
    if (_firstAppStateCall)
    {
        _firstAppStateCall = NO;
    }
    else
    {
        if (_musicPlayer != nil)
        {
            [_musicPlayer play];
        }
    }
}

///------------------------------------------------------------------------------------------------

- (void) updateAudioWith:(float) dtMillis
{
    float FADE_SPEED = 0.00125f;
    
    if (_musicPlayer != nil)
    {
        if (![_nextQueuedMusicPath isEqualToString:_currentPlayingMusicPath])
        {
            if (_musicPlayer.volume > 0.0f)
            {
                _musicPlayer.volume -= dtMillis * FADE_SPEED;
            }
            else
            {
                _musicPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:_nextQueuedMusicPath] error:nil];
                _musicPlayer.numberOfLoops = -1;
                _musicPlayer.volume = 0.0f;
                [_musicPlayer prepareToPlay];
                [_musicPlayer play];
                _currentPlayingMusicPath = _nextQueuedMusicPath;
            }
        }
        else
        {
            if (_musicPlayer.volume < _targetMusicVolume)
            {
                _musicPlayer.volume += dtMillis * FADE_SPEED;
            }
        }
    }
    
}

///------------------------------------------------------------------------------------------------

- (void) setAudioEnabledWith:(BOOL) audioEnabled
{
    _targetMusicVolume = audioEnabled ? ENABLED_AUDIO_MUSIC_VOLUME : DISABLED_AUDIO_MUSIC_VOLUME;
    _targetSfxVolume = audioEnabled ? ENABLED_AUDIO_SFX_VOLUME : DISABLED_AUDIO_SFX_VOLUME;
    
    if (_musicPlayer != nil)
    {
        _musicPlayer.volume = _targetMusicVolume;
    }
    for(id key in _sfxPlayers)
    {
        AVAudioPlayer* sfxPlayer = [_sfxPlayers objectForKey:key];
        sfxPlayer.volume = _targetSfxVolume;
    }
}

///------------------------------------------------------------------------------------------------

@end

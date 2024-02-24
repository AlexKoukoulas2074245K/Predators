///------------------------------------------------------------------------------------------------
///  SfxPlayer.mm
///  Predators
///
///  Created by Alex Koukoulas on 14/02/2024.
///-----------------------------------------------------------------------------------------------

#import <platform_utilities/SfxPlayer.h>

///------------------------------------------------------------------------------------------------
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define MAX_SFX_VOLUME 0.5f

@interface SfxPlayer (Private)
- (BOOL)initOpenAL;
- (BOOL)destroyOpenAL;
- (NSUInteger) nextAvailableSource;
- (AudioFileID) openAudioFile:(NSString*)fileName;
- (UInt32) audioFileSize:(AudioFileID)fileDescriptor;
@end

@implementation SfxPlayer

- (id) init
{
    _soundSources = [[NSMutableArray alloc] init];
    _soundLibrary = [[NSMutableDictionary alloc] init];
    _targetSfxVolume = MAX_SFX_VOLUME;
    _audioEnabled = YES;
    
    BOOL result = [self initOpenAL];
    if (!result) return nil;
    return self;
}

- (void) pauseSfx
{
    _targetSfxVolume = 0.0f;
}

- (void) resumeSfx
{
    [self initOpenAL];
    _targetSfxVolume = MAX_SFX_VOLUME;
}

- (void) setAudioEnabledWith:(BOOL)audioEnabled
{
    _audioEnabled = audioEnabled;
}

- (BOOL) initOpenAL
{
    _device = alcOpenDevice(NULL);
    
    if (_device)
    {
        _context = alcCreateContext(_device, NULL);
        alcMakeContextCurrent(_context);
        
        ALuint sourceID;
        for (int i = 0; i < kMaxSources; ++i)
        {
            alGenSources(1, &sourceID);
            
            [_soundSources addObject:[NSNumber numberWithUnsignedInt:sourceID]];
        }
        
        return YES;
    }
    
    return NO;
}

- (BOOL) destroyOpenAL
{
    for (NSNumber* sourceNumber in _soundSources)
    {
        ALuint nsInt = static_cast<ALuint>([sourceNumber unsignedIntValue]);
        alDeleteSources(1, &nsInt);
    }
    
    alcDestroyContext(_context);
    alcCloseDevice(_device);
    return YES;
}

- (void) loadSoundWithName:(NSString *)soundName filePath:(NSString *)filePath
{
    NSString* sandboxFilePath = [NSBundle.mainBundle pathForResource:filePath ofType:@"wav"];
    
    AudioFileID fileID = [self openAudioFile:sandboxFilePath];
    
    UInt32 fileSize = [self audioFileSize:fileID];
    
    void* outData = malloc(fileSize);
    
    OSStatus result = noErr;
    result = AudioFileReadBytes(fileID, FALSE, 0, &fileSize, outData);
    AudioFileClose(fileID);
    
    if (result != 0)
    {
        NSLog(@"ERROR SoundEngine: Cannot load sound: %@", filePath);
    }
    
    ALuint bufferID;
    
    alGenBuffers(1, &bufferID);
    
    alBufferData(bufferID, AL_FORMAT_STEREO16, outData, fileSize, 48000);
    
    [_soundLibrary setObject:[NSNumber numberWithUnsignedInt:bufferID] forKey:filePath];
    
    if (outData)
    {
        free(outData);
        outData = NULL;
    }
}

- (AudioFileID) openAudioFile:(NSString *)filePath
{
    AudioFileID outAFID;
    
    NSURL* afUrl = [NSURL fileURLWithPath:filePath];
    
    OSStatus result = AudioFileOpenURL((__bridge CFURLRef)afUrl, kAudioFileReadPermission, 0, &outAFID);
    
    if (result != 0)
    {
        NSLog(@"ERROR SoundEngine: Cannot open file: %@", filePath);
        return nil;
    }
    
    return outAFID;
}

- (UInt32) audioFileSize:(AudioFileID)fileDescriptor
{
    UInt64 outDataSize = 0;
    UInt32 thePropSize = sizeof(UInt64);
    OSStatus result = AudioFileGetProperty(fileDescriptor, kAudioFilePropertyAudioDataByteCount, &thePropSize, &outDataSize);
    if (result != 0)
    {
        NSLog(@"ERROR: cannot find file size");
    }
    
    return (UInt32)outDataSize;
}

- (NSUInteger) playSoundWithName:(NSString *)soundName gain:(ALfloat)gain pitch:(ALfloat)pitch shouldLoop:(BOOL)shouldLoop
{
    if (!_audioEnabled)
    {
        return 0;
    }
    
    ALenum err = alGetError();
    
    NSNumber* numVal = [_soundLibrary objectForKey:soundName];
    if (numVal == nil)
    {
        return 0;
    }
    
    ALuint bufferID = [numVal unsignedIntValue];
    ALuint sourceID = static_cast<ALuint>([self nextAvailableSource]);
    
    alSourcei(sourceID, AL_BUFFER, 0);
    alSourcei(sourceID, AL_BUFFER, bufferID);
    
    alSourcef(sourceID, AL_PITCH, pitch);
    alSourcef(sourceID, AL_GAIN, _targetSfxVolume * gain);
    
    alSourcei(sourceID, AL_LOOPING, shouldLoop ? AL_TRUE : AL_FALSE);
    
    err = alGetError();
    if (err != 0)
    {
        NSLog(@"ERROR SoundManager: %d", err);
    }
    
    alSourcePlay(sourceID);
    
    return sourceID;
}

- (NSUInteger) nextAvailableSource
{
    ALint sourceState;
    for (NSNumber* sourceNumber in _soundSources)
    {
        alGetSourcei([sourceNumber unsignedIntValue], AL_SOURCE_STATE, &sourceState);
        if (sourceState != AL_PLAYING)
        {
            return [sourceNumber unsignedIntValue];
        }
    }
    
    ALint looping;
    for (NSNumber* sourceNumber in _soundSources)
    {
        alGetSourcei(static_cast<ALuint>([sourceNumber unsignedIntValue]), AL_LOOPING, &looping);
        if (!looping)
        {
            NSUInteger sourceID = [sourceNumber unsignedIntValue];
            alSourceStop(static_cast<ALuint>(sourceID));
            return sourceID;
        }
    }
    
    NSUInteger sourceID = [[_soundSources objectAtIndex:0] unsignedIntegerValue];
    alSourceStop(static_cast<ALuint>(sourceID));
    return sourceID;
}

@end

#pragma clang diagnostic pop

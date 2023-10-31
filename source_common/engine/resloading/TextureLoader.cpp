///------------------------------------------------------------------------------------------------
///  TextureLoader.cpp
///  Predators
///
///  Created by Alex Koukoulas on 20/09/2023.
///------------------------------------------------------------------------------------------------

#include <algorithm>
#include <engine/rendering/OpenGL.h>
#include <engine/resloading/TextureLoader.h>
#include <engine/resloading/TextureResource.h>
#include <engine/utils/FileUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/OSMessageBox.h>
#include <engine/utils/StringUtils.h>
#include <fstream>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define DESKTOP_FLOW
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #undef DESKTOP_FLOW
    #else
        #define DESKTOP_FLOW
    #endif
#endif

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

void TextureLoader::VInitialize()
{
    SDL_version imgCompiledVersion;
    SDL_IMAGE_VERSION(&imgCompiledVersion);
    
    const auto* imgLinkedVersion = IMG_Linked_Version();
    
    const auto imgMajorVersionConsistency = imgCompiledVersion.major == imgLinkedVersion->major;
    const auto imgMinorVersionConsistency = imgCompiledVersion.minor == imgLinkedVersion->minor;
    const auto imgPatchConsistency = imgCompiledVersion.patch == imgLinkedVersion->patch;
    const auto imgVersionConsistency = imgMajorVersionConsistency && imgMinorVersionConsistency && imgPatchConsistency;
    
    const auto sdlImageInitFlags = IMG_INIT_PNG;
    if (!imgVersionConsistency || IMG_Init(sdlImageInitFlags) != sdlImageInitFlags)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL_image", "SDL_image was not initialized properly");
    }
    
    logging::Log(logging::LogType::INFO, "Successfully initialized SDL_image version %d.%d.%d", imgCompiledVersion.major, imgCompiledVersion.minor, imgCompiledVersion.patch);
}

///------------------------------------------------------------------------------------------------

std::unique_ptr<IResource> TextureLoader::VCreateAndLoadResource(const std::string& resourcePath) const
{
    std::ifstream file(resourcePath);
        
    if (!file.good())
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "File could not be found", resourcePath.c_str());
        return nullptr;
    }
    
    auto* sdlSurface = IMG_Load(resourcePath.c_str());
    
    if (!sdlSurface)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL_image could not load texture", IMG_GetError());
        return nullptr;
    }

    GLuint glTextureId;
    GL_CALL(glGenTextures(1, &glTextureId));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, glTextureId));
    
    int mode;
    switch (sdlSurface->format->BytesPerPixel)
    {
        case 4:
            mode = GL_RGBA;
            break;
        case 3:
            mode = GL_RGB;
            break;
        default:
            throw std::runtime_error("Image with unknown channel profile");
            break;
    }

#if !defined(DESKTOP_FLOW)
    // OpenGL ES 3.0 format shenanigans
    SDL_LockSurface(sdlSurface);
    for (int y = 0; y < sdlSurface->h; ++y)
    {
        for (int x = 0; x < sdlSurface->w; ++x)
        {
            Uint32 * const pixel = (Uint32 *) ((Uint8 *) sdlSurface->pixels + y * sdlSurface->pitch + x * sdlSurface->format->BytesPerPixel);
            *pixel = (*pixel & 0xFF000000) + ((*pixel & 0x000000FF) << 16) + (*pixel & 0x0000FF00) + ((*pixel & 0x00FF0000) >> 16);
        }
    }
    SDL_UnlockSurface(sdlSurface);
#endif
    
    GL_CALL(glTexImage2D
    (
        GL_TEXTURE_2D,
        0,
        mode,
        sdlSurface->w,
        sdlSurface->h,
        0,
        mode,
        GL_UNSIGNED_BYTE,
        sdlSurface->pixels
     ));
    
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    
    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    
    logging::Log(logging::LogType::INFO, "Loaded %s", resourcePath.c_str());
    
    const auto surfaceWidth = sdlSurface->w;
    const auto surfaceHeight = sdlSurface->h;
    
    SDL_FreeSurface(sdlSurface);
    
    return std::unique_ptr<IResource>(new TextureResource(surfaceWidth, surfaceHeight, mode, mode, glTextureId));
}



///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

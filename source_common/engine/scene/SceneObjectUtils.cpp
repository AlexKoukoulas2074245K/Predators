///------------------------------------------------------------------------------------------------
///  SceneObjectUtils.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Fonts.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

namespace scene_object_utils
{

///------------------------------------------------------------------------------------------------

math::Rectangle GetTextSceneObjectBoundingRect(const scene::SceneObject& sceneObject)
{
    math::Rectangle boundingRect;
    
    const auto& textData = std::get<scene::TextSceneObjectData>(sceneObject.mSceneObjectTypeData);
    auto fontOpt = CoreSystemsEngine::GetInstance().GetFontRepository().GetFont(textData.mFontName);
    if (!fontOpt) return boundingRect;
    
    const auto& font = fontOpt->get();
    
    float xCursor = sceneObject.mPosition.x;
    float yCursor = sceneObject.mPosition.y;
    
    float minX = xCursor;
    float minY = yCursor;
    float maxX = xCursor;
    float maxY = yCursor;
    
    for (size_t i = 0; i < textData.mText.size(); ++i)
    {
        const auto& glyph = font.FindGlyph(textData.mText[i]);
        
        float targetX = xCursor;
        float targetY = sceneObject.mPosition.y - glyph.mYOffsetPixels * sceneObject.mScale.y * 0.5f;
        
        if (targetX + glyph.mWidthPixels * sceneObject.mScale.x/2 > maxX) maxX = targetX + glyph.mWidthPixels * sceneObject.mScale.x/2;
        if (targetX - glyph.mWidthPixels * sceneObject.mScale.x/2 < minX) minX = targetX - glyph.mWidthPixels * sceneObject.mScale.x/2;
        if (targetY + glyph.mHeightPixels * sceneObject.mScale.y/2 > maxY) maxY = targetY + glyph.mHeightPixels * sceneObject.mScale.y/2;
        if (targetY - glyph.mHeightPixels * sceneObject.mScale.y/2 < minY) minY = targetY - glyph.mHeightPixels * sceneObject.mScale.y/2;
        
        if (i != textData.mText.size() - 1)
        {
            // Since each glyph is rendered with its center as the origin, we advance
            // half this glyph's width + half the next glyph's width ahead
            const auto& nextGlyph = font.FindGlyph(textData.mText[i + 1]);
            xCursor += (glyph.mWidthPixels * sceneObject.mScale.x) * 0.5f + (nextGlyph.mWidthPixels * sceneObject.mScale.x) * 0.5f;
            xCursor += glyph.mAdvancePixels * sceneObject.mScale.x;
        }
    }
    
    boundingRect.bottomLeft = glm::vec2(minX, minY);
    boundingRect.topRight = glm::vec2(maxX, maxY);
    
    return boundingRect;
}

///------------------------------------------------------------------------------------------------

}

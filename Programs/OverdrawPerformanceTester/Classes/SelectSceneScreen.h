#ifndef __SELECT_SCENE_SCREEN_H__
#define __SELECT_SCENE_SCREEN_H__

#include "BaseScreen.h"

using namespace DAVA;

class SelectSceneScreen : public BaseScreen
{
protected:
    virtual ~SelectSceneScreen()
    {
    }

public:

    SelectSceneScreen();

    virtual void LoadResources();
    virtual void UnloadResources();

protected:
    void OnStart(BaseObject* caller, void* param, void* callerData);

private:
    struct ButtonInfo
    {
        WideString caption;
        int32 tag;
        Rect rect;
        int16 data;
    };

    void ReleaseButtons(DAVA::UnorderedMap<UIButton*, ButtonInfo>& buttons);
    void OnResolutionButtonClick(BaseObject* sender, void* data, void* callerData);
    void OnTextureFormatButtonClick(BaseObject* sender, void* data, void* callerData);
    void OnChangeOverdrawButtonClick(BaseObject* sender, void* data, void* callerData);

    UIStaticText* fileNameText = nullptr;
    UIFileSystemDialog* fileSystemDialog = nullptr;
    UIStaticText* overdrawInfoMessage = nullptr;
    UIStaticText* overdrawCountLabel = nullptr;

    FilePath scenePath;

    DAVA::UnorderedMap<UIButton*, ButtonInfo> resolutionButtons;
    DAVA::UnorderedMap<UIButton*, ButtonInfo> texturePixelFormatButtons;
    DAVA::UnorderedMap<UIButton*, ButtonInfo> overdrawButtons;

    UITextFieldDelegate* inputDelegate = nullptr;

    static const Array<ButtonInfo, 4> resolutionButtonsInfo;
    static const Array<ButtonInfo, 5> texturePixelFormatButtonsInfo;
    static const Array<ButtonInfo, 2> overdrawButtonsInfo;
    static const Color Red;
    static const Color Green;
    static const float32 resolutionButtonsXOffset;
    static const float32 resolutionButtonsYOffset;
    static const float32 buttonHeight;
    static const float32 buttonWidth;
    static const float32 heigthDistanceBetweenButtons;
    static const float32 texturePixelFormatXOffset;
    static const float32 texturePixelFormatYOffset; 
    static const float32 overdrawXOffset;
    static const float32 overdrawYOffset;
};

#endif //__SELECT_SCENE_SCREEN_H__

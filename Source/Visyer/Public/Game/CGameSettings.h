/*
===========================================================================

Visyer - Community Edition by Hevedy
<https://github.com/Hevedy/Visyer-Community>

Visyer - Community Edition GPL Source Code
Copyright (C) 2019-2022 Hevedy <https://github.com/Hevedy>.

This file is part of the Game This - Community Edition GPL Source Code.


Visyer - Community Edition GPL Source Code is free software:
you can redistribute it and/or modify it under the terms of the
GNU General Public License as published bythe Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

Visyer - Community Edition GPL Source Code is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Game This - Community Edition GPL Source Code.
If not, see <http://www.gnu.org/licenses/>.


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

===========================================================================
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Math/IntPoint.h"
#include "GameFramework/GameUserSettings.h"
#include "HEVLibrarySettings.h"
#include "CGameSettings.generated.h"

//Fix for circular dependency
class UCGameInstance;


DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGSInitializedEvent );

// TODO
// Accesibility Settings
// Deaf People Pulses strike from music suits and gamepad
// 

// 

// Change this to normal object and store it inside Instance with a value
UCLASS( Blueprintable, BlueprintType )
class VISYER_API UCGameSettings : public UObject
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCGameSettings();

protected:
	// Called when the game starts
	//virtual void BeginPlay() override;

private:
	UPROPERTY()
		UCGameInstance* GameInstance;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginDestroy() override;

	// This is the actual array of settings
	//TArray<FHEVSettingMenuStruct*> SettingsArray;

	UPROPERTY( BlueprintAssignable, Category = "Hevedy|Game|Settings" )
		FGSInitializedEvent OnInitialized;

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		bool HasInitialized();

	void DefaultSettings();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Hevedy|Game|Settings" )
		bool Setup( UCGameInstance* _Instance );

	virtual bool Setup_Implementation( UCGameInstance* _Instance );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void GetSetResolutions();


	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		int32 GetUICurrectResolutionIndex();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void SetUICurrectResolutionIndex( const int32 _Index );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void GetSetLanguage();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void SetInternalUserIndex( const int32 _Index = 0);

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		bool ReadConfig();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		bool WriteConfig();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void ApplyAccessibilitySettings();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void CalibrateSettingsFromHardware();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void CalibrateKeysFromKeyboardLayout();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void ApplyEngineSettings();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void ApplyEngineResolutionSettings();

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		bool ReadBool( const FString _Class, const FString _Variable, bool& _Value, const bool _DefaultValue, const ESettingFilesList _File = ESettingFilesList::eGameSettings );

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		bool ReadInt( const FString _Class, const FString _Variable, int32& _Value, const int32 _DefaultValue, const ESettingFilesList _File = ESettingFilesList::eGameSettings, const int32 _MinValue = 0, const int32 _MaxValue = 100 );

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		bool ReadString( const FString _Class, const FString _Variable, FString& _Value, const FString _DefaultValue, const ESettingFilesList _File = ESettingFilesList::eGameSettings, const int32 _CharLimit = 99 );

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		bool WriteBool( const FString _Class, const FString _Variable, const bool _Value, const ESettingFilesList _File = ESettingFilesList::eGameSettings );

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		bool WriteInt( const FString _Class, const FString _Variable, const int32 _Value, const ESettingFilesList _File = ESettingFilesList::eGameSettings );

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		bool WriteString( const FString _Class, const FString _Variable, const FString _Value, const ESettingFilesList _File = ESettingFilesList::eGameSettings );


	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Config" )
		TArray<FIntPoint> FullscreenResolutions;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Config" )
		TArray<FIntPoint> WindowedResolutions;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		FIntPoint VideoResolutionDisplay;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		FIntPoint VideoResolution;

	UFUNCTION( BlueprintPure, Category = "Hevedy|Game|Settings" )
		TArray<FString> GetStringResolutions( TArray<int32>& _ArrayIndexes, const bool _Windowed );

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Config" )
		bool LastReadFailed;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Config" )
		bool LastWriteFailed;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 UserProfileIndex; // From generated list

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 UserInternalIndex;

	//
	// VIDEO
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 HardwareScore; // From generated list

	// -2 means user provided, -1 means current desktop one
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoResolutionIndex; // From generated list

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoResolutionDisplayIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoResolutionX;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoResolutionY;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		float VideoRenderScale; //Scale render

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoWindowMode; // Fullscreen, Window, Borderless

	// Render Overall (Minimum, Low, Normal, High, Maximum)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoQuality;

	// Effects (Light and Effects)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoQualityEffects;

	// -1 VSync, 0 No Limit, Other number equals the limit up 1000
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Video" )
		int32 VideoFPSLimit;

	//
	// AUDIO
	//
	// Audio Quality (Low, Normal, Extra)
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Audio" )
		int32 AudioQuality;

	// World Quality
	//UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Video" )
		//int32 VideoQualityWorld;


	//
	// ACCESSIBILITY
	//

	// Accessibility - 1 Wasn't executed, 0 Disabled, 1 Enabled
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Accessibility" )
		int32 AccessibilitySetup;

	// Vision: Tritanopia, Protanopia, Deuteranopia, Common, Vision Difficulties, Without Sight
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Accessibility" )
		int32 AccessibilityVision;

	// Hearing: Sensitive, Default, Hard of Hearing, Deaf
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Accessibility" )
		int32 AccessibilityHearing;

	// Mobility: Left Handed( Limited ), Left Handed, Right Handed, Right Handed( Limited )
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Accessibility" )
		int32 AccessibilityMobility;

	//
	// VISUALS
	//
	// Tritanopia, Protanopia, Deuteranopia, Common
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		int32 ColorBlindMode;

	// English, Spanish
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		int32 UILanguage;

	// Small, Default, Big
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		int32 UIPreset;

	// Complete, Only Player and Only Title, Only Title, None
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		int32 UIMode;

	// Disabled, Subtitles, Visual Indicators, Subtitles and Visual Indicators
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		int32 VisualHelpers;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		bool GammaDynamic;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		int32 Gamma;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		float CameraShakeIntensity;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		float MusicToSceneIntensity;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Visuals" )
		float CameraSpeed;


	//
	// AUDIO
	//
	// From 0.0 to 1.0
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Audio" )
		float AudioVolumeMaster;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Audio" )
		float AudioVolumeMusic;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Audio" )
		float AudioVolumeMusicBass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Audio" )
		float AudioVolumeAnnouncer;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Audio" )
		float AudioVolumeSFX;


	//
	// CONTROLS
	//

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		bool ValidateKeyBinds();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void SetActionKeysBind( const FName _Action, UInputSettings* _Input, TArray<FInputActionKeyMapping> _ActionsArray );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void SetAxisKeysBind( const FName _Axis, UInputSettings* _Input, TArray<FInputAxisKeyMapping> _AxisArray );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Settings" )
		void SetKeyBinds();


	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		int32 KeyboardLayoutIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		EKeyboardLayoutList KeyboardLayout;

	// From 0.0 to 1.0
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		float InputSensibilityHorizontal;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		float InputSensibilityVertical;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		bool InputInvertHorizontal;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		bool InputInvertVertical;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		float ForceFeedbackShakeIntensity;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		TArray<float> ForceFeedbackShakeIntensitiesHz;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveForward;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveBackward;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveRight;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveLeft;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveDepth;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveDepthAlt;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveDepthReverse;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveDepthReverseAlt;


	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KActionMain;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KActionSpecial;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveForwardAlt;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveBackwardAlt;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveRightAlt;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KMoveLeftAlt;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KActionMainAlt;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Input" )
		FKey KActionSpecialAlt;

	//
	// User
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString Username;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString UserClan;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString UserNickname;

	//
	// Social Media
	//
	// Do this better with the pre made C# app
	// Don't use Twitch or youtube but rather Discord to control the voting
	// Discord Individual
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		bool DiscordRPEnabled = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString DiscordChannel;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString DiscordName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString DiscordBotName;

	// Twitch Individual
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString TwitchChannel;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString TwitchName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString TwitchBotName;

	//
	// TRACK PLAYER
	//

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString TrackPlayerSingle;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		int32 TrackPlayerDeviceIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		int32 TrackPlayerPlaylistIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		int32 TrackPlayerTrackIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		int32 TrackPlayerAutoPlayIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		int32 TrackPlayerSceneIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		int32 TrackPlayerModeIndex;


	//
	// COMPOSER PLAYER
	//

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString ComposerPlayerSingle;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		int32 ComposerPlayerTrackIndex;


	//
	// MISC
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		float HzVisualMaster;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		TArray<float> HzVisualValues;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		FString LastDirectory;

	//  Main menu systems
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		TArray<int32> FeaturedPlayerScenes;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		TArray<int32> FeaturedComposerScenes;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Settings|Profile" )
		TArray<int32> FeaturedGameScenes;

private:

	UPROPERTY()
		bool Initialized = false;

	UFUNCTION()
		void SetCommands();
};

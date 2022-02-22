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

#include "CGameSettings.h"
#include "CGameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/InputComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "Internationalization/Internationalization.h"
#include "HEVLibraryIOParser.h"
#include "CMainBlueprintFL.h"

#define LOCTEXT_NAMESPACE "Game.Settings"

// Sets default values for this component's properties
UCGameSettings::UCGameSettings()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UCGameSettings::BeginDestroy() {
	//SaveConfig();
	//WriteConfig();

	Super::BeginDestroy();
}


bool UCGameSettings::HasInitialized() {
	return Initialized;
}

void UCGameSettings::DefaultSettings() {

}

bool UCGameSettings::Setup_Implementation( UCGameInstance* _Instance ) {
	GameInstance = _Instance;
	GetSetResolutions();
	ReadConfig();
	GetSetLanguage();
	CalibrateSettingsFromHardware();
	ApplyAccessibilitySettings();
	///CalibrateKeysFromKeyboardLayout(); //Done only on first setup
	ValidateKeyBinds(); // Validate the key binds
	SetKeyBinds(); // Apply Binds to System
	ApplyEngineResolutionSettings();
	ApplyEngineSettings();
	SetCommands();
	//ApplyEngineSettings();
	//return true;

	Initialized = true;
	OnInitialized.Broadcast();

	return true;
}

void UCGameSettings::GetSetResolutions() {
	UGameUserSettings* settings = GameInstance->GetGameUserSettings();
	VideoResolutionDisplay = settings->GetDesktopResolution();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions( FullscreenResolutions );
	UKismetSystemLibrary::GetConvenientWindowedResolutions( WindowedResolutions );

	bool valid = false;
	for ( int32 i = 0; i < FullscreenResolutions.Num(); i++ ) {
		if ( FullscreenResolutions[i] == VideoResolutionDisplay ) {
			VideoResolutionDisplayIndex = i;
			valid = true;
			break;
		}
	}

	if ( !valid ) {
		VideoResolutionDisplayIndex = FullscreenResolutions.Num() - 1;
	}
}

int32 UCGameSettings::GetUICurrectResolutionIndex() {
	if ( VideoResolutionIndex == -2 || VideoResolutionIndex == -1 ) {
		return VideoResolutionDisplayIndex;
	} else {
		return VideoResolutionIndex;
	}
}

void UCGameSettings::SetUICurrectResolutionIndex( const int32 _Index ) {
	if ( _Index == VideoResolutionDisplayIndex ) {
		VideoResolutionIndex = -1;
	} else {
		VideoResolutionIndex = FMath::Clamp( _Index, 0, FullscreenResolutions.Num() );
	}
}

void UCGameSettings::GetSetLanguage() {
	if ( UILanguage == -1 ) {
		FString lang = FInternationalization::Get().GetDefaultLanguage()->GetName();
		if ( lang.Contains( "en" ) ) {
			UILanguage = 0;
		} else if ( lang.Contains( "es" ) ) {
			UILanguage = 1;
		} else {
			UILanguage = 0;
		}
	}
	WriteInt( "Visual", "UILanguage", UILanguage, ESettingFilesList::eUserSettings );
}

void UCGameSettings::SetInternalUserIndex( const int32 _Index ) {
	int32 index = _Index;
	index = FMath::Clamp( _Index, 0, 5);
	UserProfileIndex = index;
	index = FMath::Clamp( FMath::Max( 0, index - 1 ), 0, 4 );
	UserInternalIndex = index;
}

TArray<float> HzStringToFloat( const FString _String, const int32 _Num = 10 ) {

	bool valid = false;
	TArray<int32> src = UHEVLibraryIOParser::GetIntOfStringArray( _String, valid, "|" );
	TArray<float> out;
	while ( src.Num() < _Num ) {
		src.Add( 100 ); 
	}
	for ( int32 i = 0; i < _Num; i++ ) {
		out.Add( (float)FMath::Clamp( src[i], 0, 100 ) * 0.01f );
	}
	return out;
}

FString HzFloatToString( const TArray<float> _Float ) {
	FString str = "";
	for ( int32 i = 0; i < _Float.Num(); i++ ) {
		FString lStr = FString::FromInt( FMath::TruncToInt( _Float[i] * 100 ) );
		if ( i == _Float.Num()-1 ) {
			str += lStr;
		} else {
			str += lStr + "|";
		}
	}
	return str;
}

TArray<int32> StringToInt( const FString _String, const int32 _Num = 2 ) {

	bool valid = false;
	TArray<int32> src = UHEVLibraryIOParser::GetIntOfStringArray( _String, valid, "|" );
	TArray<int32> out;
	while ( src.Num() < _Num ) {
		src.Add( 100 );
	}
	for ( int32 i = 0; i < _Num; i++ ) {
		out.Add( FMath::Clamp( src[i], 0, 100 ));
	}
	return out;
}

FString IntToString( const TArray<int32> _Int ) {
	FString str = "";
	for ( int32 i = 0; i < _Int.Num(); i++ ) {
		FString lStr = FString::FromInt( _Int[i] );
		if ( i == _Int.Num() - 1 ) {
			str += lStr;
		} else {
			str += lStr + "|";
		}
	}
	return str;
}

bool UCGameSettings::ReadConfig() {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameSettings::ReadConfig()" ) );
	bool valid = UHEVLibrarySettings::SettingsDirectoryValidate();
	if ( valid ) {

		//
		// GAME SETTINGS
		//
		ESettingFilesList curFile = ESettingFilesList::eLauncherSettings;
		FString curClass = "General";
		//FString test;
		int32 outputValue = 0;

		curClass = "Launcher";
		UHEVLibrarySettings::ReadWriteINI( HardwareScore, "HardwareScore", curClass, curFile, 0, -1, -1, 10 );
		UHEVLibrarySettings::ReadWriteINI( UserProfileIndex, "ActiveUser", curClass, curFile, 0, 0, 0, 5 );


		curFile = ESettingFilesList::eGameSettings;
		curClass = "Video";
		UHEVLibrarySettings::ReadWriteINI( VideoResolutionIndex, "ResolutionIndex", curClass, curFile, 0, -1, -2, 999 ); // -1 Native
		UHEVLibrarySettings::ReadWriteINI( VideoResolution.X, "ResolutionY", curClass, curFile, 0, 1280, 640, 999999 ); // Get current
		UHEVLibrarySettings::ReadWriteINI( VideoResolution.Y, "ResolutionY", curClass, curFile, 0, 720, 480, 999999 );
		UHEVLibrarySettings::ReadWriteINI( outputValue, "RenderScale", curClass, curFile, 0, 100, 25, 100 );
		VideoRenderScale = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( VideoWindowMode, "WindowMode", curClass, curFile, 0, 2, 0, 2 );
		UHEVLibrarySettings::ReadWriteINI( VideoQuality, "Quality", curClass, curFile, 0, 2, 0, 4 );
		UHEVLibrarySettings::ReadWriteINI( VideoQualityEffects, "QualityEffects", curClass, curFile, 0, 1, 0, 2 );
		UHEVLibrarySettings::ReadWriteINI( outputValue, "FPSLimit", curClass, curFile, 0, -1, -1, 1000 );
		if ( outputValue != -1 && outputValue != 0 ) { // Special settings for Vsyc and Unlimited
			VideoFPSLimit = FMath::Clamp( outputValue, 25, 1000 ); // Don't allow FPS under 25
		}

		curClass = "Audio";
		UHEVLibrarySettings::ReadWriteINI( AudioQuality, "Quality", curClass, curFile, 0, 2, 0, 4 );

		//
		// USER SETTINGS
		//
		UE_LOG( LogTemp, Warning, TEXT( "User Index Pre Set: %d" ), UserProfileIndex );
		SetInternalUserIndex( UserProfileIndex ); // Set user Index
		UE_LOG( LogTemp, Warning, TEXT( "User Index Post Set: %d" ), UserProfileIndex );

		curFile = ESettingFilesList::eUserSettings;
		curClass = "General";
		//FString userName;


		curClass = "Accessibility";
		UHEVLibrarySettings::ReadWriteINI( AccessibilitySetup, "Setup", curClass, curFile, UserInternalIndex, -1, 0, 1 );
		UHEVLibrarySettings::ReadWriteINI( AccessibilityVision, "Vision", curClass, curFile, UserInternalIndex, 3, 0, 5 );
		UHEVLibrarySettings::ReadWriteINI( AccessibilityHearing, "Hearing", curClass, curFile, UserInternalIndex, 1, 0, 3 );
		UHEVLibrarySettings::ReadWriteINI( AccessibilityMobility, "Mobility", curClass, curFile, UserInternalIndex, 2, 0, 3 );

		curClass = "Visual";

		UHEVLibrarySettings::ReadWriteINI( ColorBlindMode, "ColorBlindMode", curClass, curFile, UserInternalIndex, 3, 0, 3 );
		UHEVLibrarySettings::ReadWriteINI( UILanguage, "UILanguage", curClass, curFile, UserInternalIndex, -1, -1, 1 );
		UHEVLibrarySettings::ReadWriteINI( UIPreset, "UIPreset", curClass, curFile, UserInternalIndex, 1, 0, 2 );
		UHEVLibrarySettings::ReadWriteINI( VisualHelpers, "Helpers", curClass, curFile, UserInternalIndex, 0, 0, 1 );
		UHEVLibrarySettings::ReadWriteINI( outputValue, "GammaDynamic", curClass, curFile, UserInternalIndex, 0, 0, 1 );
		GammaDynamic = outputValue == 1 ? true : false;
		UHEVLibrarySettings::ReadWriteINI( Gamma, "Gamma", curClass, curFile, UserInternalIndex, 50, 25, 100 );
		UHEVLibrarySettings::ReadWriteINI( outputValue, "CameraShakeIntensity", curClass, curFile, UserInternalIndex, 50, 0, 100 );
		CameraShakeIntensity = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "MusicToSceneIntensity", curClass, curFile, UserInternalIndex, 50, 10, 100 );
		MusicToSceneIntensity = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "CameraSpeed", curClass, curFile, UserInternalIndex, 50, 10, 100 );
		CameraSpeed = (float)outputValue / 100;
		switch ( UCMainBlueprintFL::GetGameTier() ) {
			case EGameTierTypesList::ETier0:
				UHEVLibrarySettings::ReadWriteINI( UIMode, "UIMode", curClass, curFile, UserInternalIndex, 0, 0, 1 );
				break;
			case EGameTierTypesList::ETier1:
				UHEVLibrarySettings::ReadWriteINI( UIMode, "UIMode", curClass, curFile, UserInternalIndex, 0, 0, 2 );
				break;
			case EGameTierTypesList::ETier2:
				UHEVLibrarySettings::ReadWriteINI( UIMode, "UIMode", curClass, curFile, UserInternalIndex, 0, 0, 3 );
				break;
		}


		curClass = "Audio";
		UHEVLibrarySettings::ReadWriteINI( outputValue, "VolumeMaster", curClass, curFile, UserInternalIndex, 80, 0, 100 );
		AudioVolumeMaster = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "VolumeMusic", curClass, curFile, UserInternalIndex, 80, 0, 100 );
		AudioVolumeMusic = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "VolumeMusicBass", curClass, curFile, UserInternalIndex, 80, 0, 100 );
		AudioVolumeMusicBass = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "VolumeAnnouncer", curClass, curFile, UserInternalIndex, 80, 0, 100 );
		AudioVolumeAnnouncer = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "VolumeSFX", curClass, curFile, UserInternalIndex, 80, 0, 100 );
		AudioVolumeSFX = (float)outputValue / 100;


		curClass = "Control";
		UHEVLibrarySettings::ReadWriteINI( KeyboardLayoutIndex, "KeyboardLayout", curClass, curFile, UserInternalIndex, 0, 0, 4 );
		UHEVLibrarySettings::ReadWriteINI( outputValue, "SensibilityHorizontal", curClass, curFile, UserInternalIndex, 50, 10, 100 );
		InputSensibilityHorizontal = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "SensibilityVertical", curClass, curFile, UserInternalIndex, 50, 10, 100 );
		InputSensibilityVertical = (float)outputValue / 100;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "InvertHorizontal", curClass, curFile, UserInternalIndex, 1, 0, 1 );
		InputInvertHorizontal = outputValue == 1 ? true : false;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "InvertVertical", curClass, curFile, UserInternalIndex, 0, 0, 1 );
		InputInvertVertical = outputValue == 1 ? true : false;
		UHEVLibrarySettings::ReadWriteINI( outputValue, "ForceFeedbackShakeIntensity", curClass, curFile, UserInternalIndex, 50, 0, 100 );
		ForceFeedbackShakeIntensity = (float)outputValue / 100;

		CalibrateKeysFromKeyboardLayout();

		FString hzArray = "";
		FString hzArrayDefault = "100|100|100";
		UHEVLibrarySettings::ReadWriteINI( hzArray, "HzVibrationValues", curClass, curFile, UserInternalIndex, hzArrayDefault, 999 );
		ForceFeedbackShakeIntensitiesHz = HzStringToFloat( hzArray, 3 );

		FString ouputKeyValue = "";
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "Forward", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveForward = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveForward );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "Backward", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveBackward = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveBackward );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "Right", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveRight = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveRight );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "Left", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveLeft = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveLeft );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "Depth", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveDepth = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveDepth );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "DepthReverse", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveDepthReverse = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveDepthReverse );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "Action", curClass, curFile, UserInternalIndex, "", 20 );
		KActionMain = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KActionMain );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "ActionSpecial", curClass, curFile, UserInternalIndex, "", 20 );
		KActionSpecial = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KActionSpecial );

		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "ForwardAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveForwardAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveForwardAlt );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "BackwardAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveBackwardAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveBackwardAlt );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "RightAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveRightAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveRightAlt );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "LeftAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveLeftAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveLeftAlt );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "DepthAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveDepthAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveDepthAlt );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "DepthReverseAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KMoveDepthReverseAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KMoveDepthReverseAlt );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "ActionAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KActionMainAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KActionMainAlt );
		UHEVLibrarySettings::ReadWriteINI( ouputKeyValue, "ActionSpecialAlt", curClass, curFile, UserInternalIndex, "", 20 );
		KActionSpecialAlt = UHEVLibraryIOParser::GetKeyFromString( ouputKeyValue, KActionSpecialAlt );


		curClass = "User";
		UHEVLibrarySettings::ReadWriteINI( Username, "Name", curClass, curFile, UserInternalIndex, "User", 50 );
		UHEVLibrarySettings::ReadWriteINI( UserClan, "Clan", curClass, curFile, UserInternalIndex, "", 4 );
		UHEVLibrarySettings::ReadWriteINI( UserNickname, "Nickname", curClass, curFile, UserInternalIndex, "Player", 30 );

		curClass = "Social";
		UHEVLibrarySettings::ReadWriteINI( outputValue, "DiscordRP", curClass, curFile, UserInternalIndex, 1, 0, 1 );
		DiscordRPEnabled = outputValue == 1 ? true : false;
		UHEVLibrarySettings::ReadWriteINI( DiscordChannel, "DiscordChannel", curClass, curFile, UserInternalIndex, "", 99 );
		UHEVLibrarySettings::ReadWriteINI( DiscordName, "DiscordName", curClass, curFile, UserInternalIndex, "", 99 );
		UHEVLibrarySettings::ReadWriteINI( TwitchChannel, "TwitchChannel", curClass, curFile, UserInternalIndex, "", 99 );
		UHEVLibrarySettings::ReadWriteINI( TwitchName, "TwitchName", curClass, curFile, UserInternalIndex, "", 99 );
		UHEVLibrarySettings::ReadWriteINI( TwitchBotName, "TwitchBotName", curClass, curFile, UserInternalIndex, "", 99 );


		curClass = "TrackPlayer";
		UHEVLibrarySettings::ReadWriteINI( TrackPlayerSingle, "Single", curClass, curFile, UserInternalIndex, "None", 999 );
		//UHEVLibrarySettings::ReadWriteINI( TrackPlayerDeviceIndex, "Device", curClass, curFile, UserInternalIndex, 1, 0, 4 );
		switch ( UCMainBlueprintFL::GetGameTier() ) {
			case EGameTierTypesList::ETier0:
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerDeviceIndex, "Device", curClass, curFile, UserInternalIndex, 1, 0, 1 );
				break;
			case EGameTierTypesList::ETier1:
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerDeviceIndex, "Device", curClass, curFile, UserInternalIndex, 1, 0, 4 );
				break;
			case EGameTierTypesList::ETier2:
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerDeviceIndex, "Device", curClass, curFile, UserInternalIndex, 1, 0, 4 );
				break;
		}

		UHEVLibrarySettings::ReadWriteINI( TrackPlayerPlaylistIndex, "Playlist", curClass, curFile, UserInternalIndex, 0, 0, 250 );
		UHEVLibrarySettings::ReadWriteINI( TrackPlayerTrackIndex, "Track", curClass, curFile, UserInternalIndex, 0, 0, 500 );
		UHEVLibrarySettings::ReadWriteINI( TrackPlayerAutoPlayIndex, "AutoPlayMode", curClass, curFile, UserInternalIndex, 0, 0, 3 ); //No, Same Song, Loop, Random


		curClass = "Composer";
		UHEVLibrarySettings::ReadWriteINI( ComposerPlayerSingle, "ComposerSingle", curClass, curFile, UserInternalIndex, "None", 999 );
		UHEVLibrarySettings::ReadWriteINI( ComposerPlayerTrackIndex, "ComposerTrackIndex", curClass, curFile, 0, 0, 0, 100 );

		curClass = "Misc";
		switch ( UCMainBlueprintFL::GetGameTier() ) {
			case EGameTierTypesList::ETier0:
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerSceneIndex, "Scene", curClass, curFile, UserInternalIndex, 0, 0, 1 );
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerModeIndex, "Mode", curClass, curFile, UserInternalIndex, 0, 0, 2 );
				break;
			case EGameTierTypesList::ETier1:
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerSceneIndex, "Scene", curClass, curFile, UserInternalIndex, 0, 0, 10 );
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerModeIndex, "Mode", curClass, curFile, UserInternalIndex, 0, 0, 4 );
				break;
			case EGameTierTypesList::ETier2:
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerSceneIndex, "Scene", curClass, curFile, UserInternalIndex, 0, 0, 10 );
				UHEVLibrarySettings::ReadWriteINI( TrackPlayerModeIndex, "Mode", curClass, curFile, UserInternalIndex, 0, 0, 4 );
				break;
		}

		UHEVLibrarySettings::ReadWriteINI( outputValue, "HzVisualMaster", curClass, curFile, UserInternalIndex, 50, 0, 100 );
		HzVisualMaster = (float)outputValue / 100;
		hzArray = "";
		hzArrayDefault = "100|100|100|100|100|100|100|100|100|100|100|100|100|100|100|100|100|100|100|100";
		UHEVLibrarySettings::ReadWriteINI( hzArray, "HzVisualValues", curClass, curFile, UserInternalIndex, hzArrayDefault, 999 );
		HzVisualValues = HzStringToFloat( hzArray, 20 );

		UHEVLibrarySettings::ReadWriteINI( LastDirectory, "LastDirectory", curClass, curFile, UserInternalIndex, "", 999 );

		hzArray = "";
		hzArrayDefault = "100|100|100";
		UHEVLibrarySettings::ReadWriteINI( hzArray, "FeaturedPlayerScenes", curClass, curFile, UserInternalIndex, hzArrayDefault, 100 );
		FeaturedPlayerScenes = StringToInt( hzArray, 3);
		hzArray = "";
		hzArrayDefault = "100|100|100";
		UHEVLibrarySettings::ReadWriteINI( hzArray, "FeaturedComposerScenes", curClass, curFile, UserInternalIndex, hzArrayDefault, 100 );
		FeaturedComposerScenes = StringToInt( hzArray, 3 );
		hzArray = "";
		hzArrayDefault = "100|100|100";
		UHEVLibrarySettings::ReadWriteINI( hzArray, "FeaturedGameScenes", curClass, curFile, UserInternalIndex, hzArrayDefault, 100 );
		FeaturedGameScenes = StringToInt( hzArray, 3 );



		UE_LOG( LogTemp, Warning, TEXT( "Success at Read Settings" ) );
		return true;
	} else {
		UE_LOG( LogTemp, Warning, TEXT( "Failure at Read Settings" ) );
		return false;
	}
}

bool UCGameSettings::WriteConfig() {
	//
	// GAME SETTINGS
	//
	ESettingFilesList curFile = ESettingFilesList::eLauncherSettings;
	FString curClass = "General";
	int32 outputValue = 0;
	TArray<FHEVSettingsCommentStruct> comments;

	curClass = "Launcher";
	comments.Add( FHEVSettingsCommentStruct( curClass, "HardwareScore", NSLOCTEXT( "Settings", "CLauncher", "My Nice Comment Here" ).ToString() ) );
	UHEVLibrarySettings::WriteINI( HardwareScore, "HardwareScore", curClass, curFile, 0 );
	UHEVLibrarySettings::WriteINI( UserProfileIndex, "ActiveUser", curClass, curFile, 0 );

	// File, Comments
	UHEVLibrarySettings::SetINIComments( UHEVLibrarySettings::GetINIFileNameFString( curFile, 0 ), comments );
	comments.Empty();

	curFile = ESettingFilesList::eGameSettings;
	curClass = "Video";
	UHEVLibrarySettings::WriteINI( VideoResolutionIndex, "ResolutionIndex", curClass, curFile, 0 ); // -1 Native
	UHEVLibrarySettings::WriteINI( VideoResolution.X, "ResolutionX", curClass, curFile, 0 ); // Get current
	UHEVLibrarySettings::WriteINI( VideoResolution.Y, "ResolutionY", curClass, curFile, 0 );
	UHEVLibrarySettings::WriteINI( (int32)( VideoRenderScale * 100 ), "RenderScale", curClass, curFile, 0 );
	UHEVLibrarySettings::WriteINI( VideoWindowMode, "WindowMode", curClass, curFile, 0 );
	UHEVLibrarySettings::WriteINI( VideoQuality, "Quality", curClass, curFile, 0 );
	UHEVLibrarySettings::WriteINI( VideoQualityEffects, "QualityEffects", curClass, curFile, 0 );
	UHEVLibrarySettings::WriteINI( VideoFPSLimit, "FPSLimit", curClass, curFile, 0 );

	curClass = "Audio";
	UHEVLibrarySettings::WriteINI( AudioQuality, "Quality", curClass, curFile, 0 );

	// File, Comments
	UHEVLibrarySettings::SetINIComments( UHEVLibrarySettings::GetINIFileNameFString( curFile, 0 ), comments );
	comments.Empty();

	//
	// USER SETTINGS
	//
	curFile = ESettingFilesList::eUserSettings;
	//curFile = "UserSettings";
	curClass = "General";


	curClass = "Accessibility";
	comments.Add( FHEVSettingsCommentStruct( curClass, "Setup", "Set the setup" ) );
	UHEVLibrarySettings::WriteINI( AccessibilitySetup, "Setup", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( AccessibilityVision, "Vision", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( AccessibilityHearing, "Hearing", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( AccessibilityMobility, "Mobility", curClass, curFile, UserInternalIndex );

	curClass = "Visual";

	UHEVLibrarySettings::WriteINI( ColorBlindMode, "ColorBlindMode", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( UILanguage, "UILanguage", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( UIPreset, "UIPreset", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( VisualHelpers, "Helpers", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( ( GammaDynamic ? 1 : 0 ), "GammaDynamic", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( Gamma, "Gamma", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( CameraShakeIntensity * 100 ), "CameraShakeIntensity", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( MusicToSceneIntensity * 100 ), "MusicToSceneIntensity", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( CameraSpeed * 100 ), "CameraSpeed", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( UIMode, "UIMode", curClass, curFile, UserInternalIndex );

	curClass = "Audio";
	UHEVLibrarySettings::WriteINI( (int32)( AudioVolumeMaster * 100 ), "VolumeMaster", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( AudioVolumeMusic * 100 ), "VolumeMusic", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( AudioVolumeMusicBass * 100 ), "VolumeMusicBass", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( AudioVolumeAnnouncer * 100 ), "VolumeAnnouncer", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( AudioVolumeSFX * 100 ), "VolumeSFX", curClass, curFile, UserInternalIndex );


	curClass = "Control";
	UHEVLibrarySettings::WriteINI( KeyboardLayoutIndex, "KeyboardLayout", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( InputSensibilityHorizontal * 100 ), "SensibilityHorizontal", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( InputSensibilityVertical * 100 ), "SensibilityVertical", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( ( InputInvertHorizontal ? 1 : 0 ), "InvertHorizontal", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( ( InputInvertVertical ? 1 : 0 ), "InvertVertical", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( ForceFeedbackShakeIntensity * 100 ), "ForceFeedbackShakeIntensity", curClass, curFile, UserInternalIndex );

	UHEVLibrarySettings::WriteINI( HzFloatToString( ForceFeedbackShakeIntensitiesHz ), "HzVibrationValues", curClass, curFile, UserInternalIndex );

	UHEVLibrarySettings::WriteINI( KMoveForward.ToString(), "Forward", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveBackward.ToString(), "Backward", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveRight.ToString(), "Right", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveLeft.ToString(), "Left", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveDepth.ToString(), "Depth", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveDepthReverse.ToString(), "DepthReverse", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KActionMain.ToString(), "Action", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KActionSpecial.ToString(), "ActionSpecial", curClass, curFile, UserInternalIndex );

	UHEVLibrarySettings::WriteINI( KMoveForwardAlt.ToString(), "ForwardAlt", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveBackwardAlt.ToString(), "BackwardAlt", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveRightAlt.ToString(), "RightAlt", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveLeftAlt.ToString(), "LeftAlt", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveDepthAlt.ToString(), "DepthAlt", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KMoveDepthReverseAlt.ToString(), "DepthReverseAlt", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KActionMainAlt.ToString(), "ActionAlt", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( KActionSpecialAlt.ToString(), "ActionSpecialAlt", curClass, curFile, UserInternalIndex );

	curClass = "User";
	UHEVLibrarySettings::WriteINI( Username, "Name", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( UserClan, "Clan", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( UserNickname, "Nickname", curClass, curFile, UserInternalIndex );

	curClass = "Social";
	UHEVLibrarySettings::WriteINI( ( DiscordRPEnabled ? 1 : 0 ), "DiscordRP", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( DiscordChannel, "DiscordChannel", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( DiscordName, "DiscordName", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TwitchChannel, "TwitchChannel", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TwitchName, "TwitchName", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TwitchBotName, "TwitchBotName", curClass, curFile, UserInternalIndex );


	curClass = "TrackPlayer";
	UHEVLibrarySettings::WriteINI( TrackPlayerSingle, "Single", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TrackPlayerDeviceIndex, "Device", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TrackPlayerPlaylistIndex, "Playlist", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TrackPlayerTrackIndex, "Track", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TrackPlayerAutoPlayIndex, "AutoPlayMode", curClass, curFile, UserInternalIndex );

	curClass = "Misc";
	UHEVLibrarySettings::WriteINI( TrackPlayerSceneIndex, "Scene", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( TrackPlayerModeIndex, "Mode", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( (int32)( HzVisualMaster * 100 ), "HzVisualMaster", curClass, curFile, UserInternalIndex );
	UHEVLibrarySettings::WriteINI( HzFloatToString( HzVisualValues ), "HzVisualValues", curClass, curFile, UserInternalIndex );

	UHEVLibrarySettings::WriteINI( LastDirectory, "LastDirectory", curClass, curFile, UserInternalIndex );

	// File, Comments
	UHEVLibrarySettings::SetINIComments( UHEVLibrarySettings::GetINIFileNameFString( curFile, UserInternalIndex ), comments );
	comments.Empty();

	UE_LOG( LogTemp, Warning, TEXT( "Success at Write Settings" ) );
	return true;
}

bool UCGameSettings::ReadBool( const FString _Class, const FString _Variable, bool& _Value, const bool _DefaultValue, const ESettingFilesList _File ) {
	return UHEVLibrarySettings::ReadWriteINI( _Value, _Variable, _Class, _File, UserInternalIndex, _DefaultValue );
	//return false;
}

bool UCGameSettings::ReadInt( const FString _Class, const FString _Variable, int32& _Value, const int32 _DefaultValue, const ESettingFilesList _File, const int32 _MinValue, const int32 _MaxValue ) {
	return UHEVLibrarySettings::ReadWriteINI( _Value, _Variable, _Class, _File, UserInternalIndex, _DefaultValue, _MinValue, _MaxValue );
	//return false;
}

bool UCGameSettings::ReadString( const FString _Class, const FString _Variable, FString& _Value, const FString _DefaultValue, const ESettingFilesList _File, const int32 _CharLimit ) {
	//return false;
	return UHEVLibrarySettings::ReadWriteINI( _Value, _Variable, _Class, _File, UserInternalIndex, _DefaultValue, _CharLimit );
}

bool UCGameSettings::WriteBool( const FString _Class, const FString _Variable, const bool _Value, const ESettingFilesList _File ) {
	return UHEVLibrarySettings::WriteINI( _Value, _Variable, _Class, _File, UserInternalIndex );
}

bool UCGameSettings::WriteInt( const FString _Class, const FString _Variable, const int32 _Value, const ESettingFilesList _File ) {
	UE_LOG( LogTemp, Warning, TEXT( "Saved: %s - %d" ), *_Variable, _Value );
	return UHEVLibrarySettings::WriteINI( _Value, _Variable, _Class, _File, UserInternalIndex );
}

bool UCGameSettings::WriteString( const FString _Class, const FString _Variable, const FString _Value, const ESettingFilesList _File ) {
	return UHEVLibrarySettings::WriteINI( _Value, _Variable, _Class, _File, UserInternalIndex );
}


TArray<FString> UCGameSettings::GetStringResolutions( TArray<int32>& _ArrayIndexes, const bool _Windowed ) {
	TArray<FString> list;
	TArray<int32> listIndex;
	int32 count = 0;
	if ( _Windowed ) {
		for ( const FIntPoint iPoint : WindowedResolutions ) {
			list.Add( FString::FromInt( iPoint.X ) + "x" + FString::FromInt( iPoint.Y ) );
			listIndex.Add( count );
			count++;
		}
	} else {
		for ( const FIntPoint iPoint : FullscreenResolutions ) {
			list.Add( FString::FromInt( iPoint.X ) + "x" + FString::FromInt( iPoint.Y ) );
			listIndex.Add( count );
			count++;
		}
	}
	_ArrayIndexes = listIndex;
	return list;
}

void UCGameSettings::ApplyAccessibilitySettings() {
	if ( AccessibilitySetup == 0 ) {
		return;
	} else if ( AccessibilitySetup == -1 ) {

	} else {
		if ( UserProfileIndex != 0 ) {
			return;
		}
	}
	// VISION
	switch ( AccessibilityVision ) {
		case 0: //Tritanopia
			ColorBlindMode = 0;
			GammaDynamic = false;
			Gamma = 40;
			VideoFPSLimit = 60;
			CameraShakeIntensity = 25;
			break;
		case 1: //Protanopia
			ColorBlindMode = 1;
			GammaDynamic = false;
			Gamma = 40;
			VideoFPSLimit = 60;
			CameraShakeIntensity = 25;
			break;
		case 2: //Deuteranopia
			ColorBlindMode = 2;
			GammaDynamic = false;
			Gamma = 40;
			VideoFPSLimit = 60;
			CameraShakeIntensity = 25;
			break;
		case 3: //Common
			ColorBlindMode = 3;
			GammaDynamic = false;
			Gamma = 50;
			break;
		case 4: //Vision Difficulties
			ColorBlindMode = 3;
			GammaDynamic = false;
			Gamma = 40;
			VideoFPSLimit = 30;
			UIPreset = 2;
			MusicToSceneIntensity = 75;
			CameraSpeed = 25;
			CameraShakeIntensity = 0;
			break;
		case 5: //Without Sight
			ColorBlindMode = 3;
			GammaDynamic = false;
			Gamma = 80;
			VideoFPSLimit = 30;
			CameraShakeIntensity = 0;
			break;
		default:
			ColorBlindMode = 3;
			GammaDynamic = false;
			Gamma = 50;
			break;
	}

	// HEARING
	switch ( AccessibilityHearing ) {
		case 0: //Sensitive
			AudioVolumeMaster = 0.6f;
			AudioVolumeMusic = 0.6f;
			AudioVolumeMusicBass = 0.4f;
			AudioVolumeAnnouncer = 0.5f;
			AudioVolumeSFX = 0.4f;
			ForceFeedbackShakeIntensity = 0.0f;
			VisualHelpers = 1;
			break;
		case 1: //Default
			AudioVolumeMaster = 0.8f;
			AudioVolumeMusic = 0.8f;
			AudioVolumeMusicBass = 0.8f;
			AudioVolumeAnnouncer = 0.8f;
			AudioVolumeSFX = 0.8f;
			ForceFeedbackShakeIntensity = 0.0f;
			VisualHelpers = 1;
			break;
		case 2: //Hard of Hearing
			AudioVolumeMaster = 1.0f;
			AudioVolumeMusic = 0.9f;
			AudioVolumeMusicBass = 0.8f;
			AudioVolumeAnnouncer = 1.0f;
			AudioVolumeSFX = 0.9f;
			ForceFeedbackShakeIntensity = 0.2f;
			VisualHelpers = 3;
			break;
		case 3: //Deaf
			AudioVolumeMaster = 0.0f;
			AudioVolumeMusic = 0.0f;
			AudioVolumeMusicBass = 0.0f;
			AudioVolumeAnnouncer = 0.0f;
			AudioVolumeSFX = 0.0f;
			ForceFeedbackShakeIntensity = 0.8f;
			VisualHelpers = 3;
			break;
		default:
			AudioVolumeMaster = 0.8f;
			AudioVolumeMusic = 0.8f;
			AudioVolumeMusicBass = 0.8f;
			AudioVolumeAnnouncer = 0.8f;
			AudioVolumeSFX = 0.8f;
			ForceFeedbackShakeIntensity = 0.0f;
			VisualHelpers = 1;
			break;
	}

	// MOBILITY
	switch ( AccessibilityMobility ) {
		case 0: //Left Handed( Limited )
			InputSensibilityHorizontal = 0.8f;
			InputSensibilityVertical = 0.7f;
			break;
		case 1: //Left Handed
			InputSensibilityHorizontal = 0.5f;
			InputSensibilityVertical = 0.5f;
			break;
		case 2: //Right Handed
			InputSensibilityHorizontal = 0.5f;
			InputSensibilityVertical = 0.5f;
			break;
		case 3: //Right Handed( Limited )
			InputSensibilityHorizontal = 0.8f;
			InputSensibilityVertical = 0.7f;
			break;
		default:
			InputSensibilityHorizontal = 0.5f;
			InputSensibilityVertical = 0.5f;
			break;
	}

	if ( AccessibilitySetup == -1 ) {
		AccessibilitySetup = 1;
	}

	//WriteConfig();
}

void UCGameSettings::CalibrateSettingsFromHardware() {
	if ( HardwareScore == -1 ) {
		HardwareScore = 5;
		//Find a better way
	} else {
		if ( UserProfileIndex != 0 ) {
			return;
		}
	}
	switch ( HardwareScore ) {
		case 0:
			VideoRenderScale = 0.25f;
			VideoQuality = 0;
			VideoQualityEffects = 0;
			break;
		case 1:
			VideoRenderScale = 0.5f;
			VideoQuality = 0;
			VideoQualityEffects = 0;
			break;
		case 2:
			VideoRenderScale = 0.75f;
			VideoQuality = 1;
			VideoQualityEffects = 0;
			break;
		case 3:
			VideoRenderScale = 0.9f;
			VideoQuality = 1;
			VideoQualityEffects = 1;
			break;
		case 4:
			VideoRenderScale = 0.9f;
			VideoQuality = 2;
			VideoQualityEffects = 1;
			break;
		case 5:
			VideoRenderScale = 1.0f;
			VideoQuality = 2;
			VideoQualityEffects = 1;
			break;
		case 6:
			VideoRenderScale = 1.0f;
			VideoQuality = 2;
			VideoQualityEffects = 1;
			break;
		case 7:
			VideoRenderScale = 1.0f;
			VideoQuality = 2;
			VideoQualityEffects = 2;
			break;
		case 8:
			VideoRenderScale = 1.0f;
			VideoQuality = 3;
			VideoQualityEffects = 1;
			break;
		case 9:
			VideoRenderScale = 1.0f;
			VideoQuality = 3;
			VideoQualityEffects = 2;
			break;
		case 10:
			VideoRenderScale = 1.0f;
			VideoQuality = 4;
			VideoQualityEffects = 2;
			break;
		default: // 5
			VideoRenderScale = 1.0f;
			VideoQuality = 2;
			VideoQualityEffects = 1;
			break;
	}

	FString fileClass = "Launcher";
	WriteInt( fileClass, "HardwareScore", HardwareScore, ESettingFilesList::eLauncherSettings );
}

void UCGameSettings::CalibrateKeysFromKeyboardLayout() {
	bool leftHanded = false;
	bool limitedMovility = false;

	switch ( AccessibilityMobility ) {
		case 0:
			leftHanded = true;
			limitedMovility = true;
		case 1:
			leftHanded = true;
			limitedMovility = false;
		case 2:
			leftHanded = false;
			limitedMovility = false;
		case 3:
			leftHanded = false;
			limitedMovility = true;
		default:
			break;
	}

	switch ( KeyboardLayoutIndex ) {
		case 0: //eQWERTY
			KeyboardLayout = EKeyboardLayoutList::eQWERTY;
			break;
		case 1: //eQWERTZ
			KeyboardLayout = EKeyboardLayoutList::eQWERTZ;
			break;
		case 2: //eAZERTY
			KeyboardLayout = EKeyboardLayoutList::eAZERTY;
			break;
		case 3: //eDvorak
			KeyboardLayout = EKeyboardLayoutList::eDvorak;
			break;
		case 4: //eColemak
			KeyboardLayout = EKeyboardLayoutList::eColemak;
			break;
		default: //eQWERTY
			KeyboardLayout = EKeyboardLayoutList::eQWERTY;
			break;
	}
	switch ( KeyboardLayout ) {
		case EKeyboardLayoutList::eQWERTY: //eQWERTY
			if ( !leftHanded ) {
				KMoveForward = EKeys::W;
				KMoveBackward = EKeys::S;
				KMoveRight = EKeys::D;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			} else {
				KMoveForward = EKeys::W;
				KMoveBackward = EKeys::S;
				KMoveRight = EKeys::D;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			}
			break;
		case EKeyboardLayoutList::eQWERTZ: //eQWERTZ
			if ( !leftHanded ) {
				KMoveForward = EKeys::W;
				KMoveBackward = EKeys::S;
				KMoveRight = EKeys::D;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			} else {
				KMoveForward = EKeys::W;
				KMoveBackward = EKeys::S;
				KMoveRight = EKeys::D;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			}
			break;
		case EKeyboardLayoutList::eAZERTY: //eAZERTY
			if ( !leftHanded ) {
				KMoveForward = EKeys::Z;
				KMoveBackward = EKeys::S;
				KMoveRight = EKeys::D;
				KMoveLeft = EKeys::Q;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			} else {
				KMoveForward = EKeys::Z;
				KMoveBackward = EKeys::S;
				KMoveRight = EKeys::D;
				KMoveLeft = EKeys::Q;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			}
			break;
		case EKeyboardLayoutList::eDvorak: //eDvorak
			if ( !leftHanded ) {
				KMoveForward = EKeys::Comma;
				KMoveBackward = EKeys::O;
				KMoveRight = EKeys::E;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			} else {
				KMoveForward = EKeys::Comma;
				KMoveBackward = EKeys::O;
				KMoveRight = EKeys::E;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			}
			break;
		case EKeyboardLayoutList::eColemak: //eColemak
			if ( !leftHanded ) {
				KMoveForward = EKeys::W;
				KMoveBackward = EKeys::R;
				KMoveRight = EKeys::S;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			} else {
				KMoveForward = EKeys::W;
				KMoveBackward = EKeys::R;
				KMoveRight = EKeys::S;
				KMoveLeft = EKeys::A;
				KActionMain = EKeys::LeftMouseButton;
				KActionSpecial = EKeys::SpaceBar;
			}
			break;
	}

	KMoveDepth = EKeys::MouseWheelAxis;
	KMoveDepthReverse = EKeys::MouseWheelAxis;

	if ( !leftHanded ) {
		if ( !limitedMovility ) {
			KMoveForwardAlt = EKeys::Gamepad_LeftY;
			KMoveBackwardAlt = EKeys::Gamepad_LeftY;
			KMoveRightAlt = EKeys::Gamepad_LeftX;
			KMoveLeftAlt = EKeys::Gamepad_LeftX;
			KActionMainAlt = EKeys::Gamepad_FaceButton_Bottom;
			KActionSpecialAlt = EKeys::Gamepad_FaceButton_Top;
		} else {
			KMoveForwardAlt = EKeys::Gamepad_LeftY;
			KMoveBackwardAlt = EKeys::Gamepad_LeftY;
			KMoveRightAlt = EKeys::Gamepad_LeftX;
			KMoveLeftAlt = EKeys::Gamepad_LeftX;
			KActionMainAlt = EKeys::Gamepad_DPad_Down;
			KActionSpecialAlt = EKeys::Gamepad_DPad_Up;
		}
		KMoveDepthAlt = EKeys::Gamepad_RightY;
		KMoveDepthReverseAlt = EKeys::Gamepad_RightY;
	} else {
		if ( !limitedMovility ) {
			KMoveForwardAlt = EKeys::Gamepad_RightY;
			KMoveBackwardAlt = EKeys::Gamepad_RightY;
			KMoveRightAlt = EKeys::Gamepad_RightX;
			KMoveLeftAlt = EKeys::Gamepad_RightX;
			KActionMainAlt = EKeys::Gamepad_DPad_Down;
			KActionSpecialAlt = EKeys::Gamepad_DPad_Up;
		} else {
			KMoveForwardAlt = EKeys::Gamepad_RightY;
			KMoveBackwardAlt = EKeys::Gamepad_RightY;
			KMoveRightAlt = EKeys::Gamepad_RightX;
			KMoveLeftAlt = EKeys::Gamepad_RightX;
			KActionMainAlt = EKeys::Gamepad_FaceButton_Bottom;
			KActionSpecialAlt = EKeys::Gamepad_FaceButton_Top;
		}
		KMoveDepthAlt = EKeys::Gamepad_LeftY;
		KMoveDepthReverseAlt = EKeys::Gamepad_LeftY;
	}
}


void UCGameSettings::ApplyEngineSettings() {
	if ( GameInstance == nullptr ) {
		return;
	}
	UGameUserSettings* settings = GameInstance->GetGameUserSettings();
	//UGameUserSettings* settings = GEngine->GetGameUserSettings();
	if ( settings == nullptr ) {
		return;
	}


	switch ( VideoQuality ) {
		// Low, Medium, High
		case 0: // Minimum
			settings->ScalabilityQuality.SetPostProcessQuality( 0 );
			settings->ScalabilityQuality.SetEffectsQuality( 0 );
			settings->ScalabilityQuality.SetShadowQuality( 0 );
			settings->ScalabilityQuality.SetViewDistanceQuality( 0 );
			settings->ScalabilityQuality.SetFoliageQuality( 0 );
			break;
		case 1: // Low
			settings->ScalabilityQuality.SetPostProcessQuality( 1 );
			settings->ScalabilityQuality.SetEffectsQuality( 1 );
			settings->ScalabilityQuality.SetShadowQuality( 1 );
			settings->ScalabilityQuality.SetViewDistanceQuality( 0 );
			settings->ScalabilityQuality.SetFoliageQuality( 0 );
			break;
		case 2: // Normal
			settings->ScalabilityQuality.SetPostProcessQuality( 1 );
			settings->ScalabilityQuality.SetEffectsQuality( 1 );
			settings->ScalabilityQuality.SetShadowQuality( 1 );
			settings->ScalabilityQuality.SetViewDistanceQuality( 1 );
			settings->ScalabilityQuality.SetFoliageQuality( 1 );
			break;
		case 3: // High
			settings->ScalabilityQuality.SetPostProcessQuality( 2 );
			settings->ScalabilityQuality.SetEffectsQuality( 1 );
			settings->ScalabilityQuality.SetShadowQuality( 2 );
			settings->ScalabilityQuality.SetViewDistanceQuality( 1 );
			settings->ScalabilityQuality.SetFoliageQuality( 1 );
			break;
		case 4: // Maximum
			settings->ScalabilityQuality.SetPostProcessQuality( 2 );
			settings->ScalabilityQuality.SetEffectsQuality( 2 );
			settings->ScalabilityQuality.SetShadowQuality( 2 );
			settings->ScalabilityQuality.SetViewDistanceQuality( 2 );
			settings->ScalabilityQuality.SetFoliageQuality( 2 );
			break;
		default:
			break;
	}

	switch ( VideoQualityEffects ) {
		case 0: // Low
			settings->ScalabilityQuality.SetAntiAliasingQuality( 0 );
			settings->ScalabilityQuality.SetTextureQuality( 0 );
			break;
		case 1: // Normal
			settings->ScalabilityQuality.SetAntiAliasingQuality( 1 );
			settings->ScalabilityQuality.SetTextureQuality( 1 );
			break;
		case 2: // High
			settings->ScalabilityQuality.SetAntiAliasingQuality( 2 );
			settings->ScalabilityQuality.SetTextureQuality( 2 );
			break;
		default:
			break;
	}

	settings->ApplyNonResolutionSettings();
	settings->SaveSettings();

	FString fileClass = "Video";
	WriteInt( fileClass, "Quality", VideoQuality, ESettingFilesList::eGameSettings );
	WriteInt( fileClass, "QualityEffects", VideoQualityEffects, ESettingFilesList::eGameSettings );

}

void UCGameSettings::ApplyEngineResolutionSettings() {
	if ( GameInstance == nullptr ) {
		return;
	}
	UGameUserSettings* settings = GameInstance->GetGameUserSettings();
	//UGameUserSettings* settings = GEngine->GetGameUserSettings();
	if ( settings == nullptr ) {
		return;
	}
	settings->ConfirmVideoMode();


	// Set FPS Limit
	if ( VideoFPSLimit == -1 ) {
		settings->SetFrameRateLimit( 0 );
		settings->SetVSyncEnabled( true );
	} else if ( VideoFPSLimit == 0 ) {
		settings->SetFrameRateLimit( 0 );
		settings->SetVSyncEnabled( false );
	} else {
		settings->SetFrameRateLimit( VideoFPSLimit );
		settings->SetVSyncEnabled( false );
	}

	EWindowMode::Type wM = EWindowMode::Type::Fullscreen;
	switch ( VideoWindowMode ) {
		case 0:
			wM = EWindowMode::Type::Fullscreen;
			break;
		case 1:
			wM = EWindowMode::Type::Windowed;
			break;
		case 2:
			wM = EWindowMode::Type::WindowedFullscreen;
			break;
		default:
			wM = EWindowMode::Type::WindowedFullscreen;
			break;
	}

	// Set Resolution
	if ( VideoResolutionIndex == -2 ) {
		// Use loaded ones
		VideoResolution.X = FMath::Min( VideoResolution.X, settings->GetDesktopResolution().X );
		VideoResolution.Y = FMath::Min( VideoResolution.Y, settings->GetDesktopResolution().Y );
	} else if ( VideoResolutionIndex == -1 ) {
		VideoResolution.X = settings->GetDesktopResolution().X;
		VideoResolution.Y = settings->GetDesktopResolution().Y;
		//settings->getsupp
	} else {
		switch ( wM ) {
			case EWindowMode::Fullscreen:
				VideoResolutionIndex = FMath::Min( VideoResolutionIndex, FMath::Max( 0, FullscreenResolutions.Num() - 1 ) );
				VideoResolution.X = FullscreenResolutions[VideoResolutionIndex].X;
				VideoResolution.Y = FullscreenResolutions[VideoResolutionIndex].Y;
				break;
			case EWindowMode::WindowedFullscreen:
				VideoResolutionIndex = FMath::Min( VideoResolutionIndex, FMath::Max( 0, FullscreenResolutions.Num() - 1 ) );
				VideoResolution.X = FullscreenResolutions[VideoResolutionIndex].X;
				VideoResolution.Y = FullscreenResolutions[VideoResolutionIndex].Y;
				break;
			case EWindowMode::Windowed:
				VideoResolutionIndex = FMath::Min( VideoResolutionIndex, FMath::Max( 0, WindowedResolutions.Num() - 1 ) );
				VideoResolution.X = WindowedResolutions[VideoResolutionIndex].X;
				VideoResolution.Y = WindowedResolutions[VideoResolutionIndex].Y;
				break;
			default:
				break;
		}
	}

	//settings->ScalabilityQuality.EffectsQuality
	settings->SetScreenResolution( FIntPoint( VideoResolution.X, VideoResolution.Y ) );
	settings->SetFullscreenMode( wM );
	settings->SetResolutionScaleNormalized( VideoRenderScale );
	//UE_LOG( LogTemp, Warning, TEXT( "Render Scale %f" ), VideoRenderScale );
	//settings->SetResolutionScaleValue( VideoRenderScale );
	//settings->SetFullscreenMode( wM );

	settings->ApplyResolutionSettings( false );
	settings->ConfirmVideoMode();
	settings->SaveSettings();


	FString fileClass = "Video";
	WriteInt( fileClass, "ResolutionX", VideoResolution.X, ESettingFilesList::eGameSettings );
	WriteInt( fileClass, "ResolutionY", VideoResolution.Y, ESettingFilesList::eGameSettings );
	WriteInt( fileClass, "ResolutionIndex", VideoResolutionIndex, ESettingFilesList::eGameSettings );

	WriteInt( fileClass, "WindowMode", VideoWindowMode, ESettingFilesList::eGameSettings );
	WriteInt( fileClass, "RenderScale", (int32)( VideoRenderScale * 100 ), ESettingFilesList::eGameSettings );

	WriteInt( fileClass, "FPSLimit", VideoFPSLimit, ESettingFilesList::eGameSettings );

}


void UCGameSettings::SetCommands() {

}

bool UCGameSettings::ValidateKeyBinds() {
	TArray<FKey> keyList;

	// Camera and Movement
	keyList.Add( KMoveForward );
	keyList.Add( KMoveBackward );

	keyList.Add( KMoveRight );
	keyList.Add( KMoveLeft );

	keyList.Add( KActionMain ); // Weapon Shoot / Stop Play music
	keyList.Add( KActionSpecial ); //Secondary game action / Change HUD

	// Gamepad
	keyList.Add( KMoveForwardAlt );
	keyList.Add( KMoveBackwardAlt );

	keyList.Add( KMoveRightAlt );
	keyList.Add( KMoveLeftAlt );

	keyList.Add( KActionMainAlt );
	keyList.Add( KActionSpecialAlt );

	TArray<int32> status;
	for ( int32 ii = 0; ii < keyList.Num(); ii++ ) {
		if ( status.Contains( ii ) ) continue;

		for ( int32 i = 0; i < keyList.Num(); i++ ) {
			if ( i == ii ) continue;

			if ( keyList[ii] == keyList[i] ) {
				status.Add( i );
			}
		}
	}

	if ( status.Num() != 0 ) {
		return true;
	} else {
		CalibrateKeysFromKeyboardLayout();
		return false;
	}

}

void UCGameSettings::SetActionKeysBind( const FName _Action, UInputSettings* _Input, TArray<FInputActionKeyMapping> _ActionsArray ) {
	if ( _Action == "" ) return;


	FInputActionKeyMapping localMap;
	TArray<FInputActionKeyMapping> fixedArray;
	for ( int32 i = 0; i < _ActionsArray.Num(); i++ ) {
		localMap = _ActionsArray[i];
		localMap.ActionName = _Action;
		fixedArray.Add( localMap );
	}

	TArray<FInputActionKeyMapping> mappings;
	if ( _Input->GetActionMappings().Num() > 0 ) {
		_Input->GetActionMappingByName( _Action, mappings );

		for ( const FInputActionKeyMapping map : mappings ) {
			_Input->RemoveActionMapping( map, false );
		}
	}

	for ( const FInputActionKeyMapping map : fixedArray ) {
		_Input->AddActionMapping( map, false );

	}
}


void UCGameSettings::SetAxisKeysBind( const FName _Axis, UInputSettings* _Input, TArray<FInputAxisKeyMapping> _AxisArray ) {
	if ( _Axis == "" ) return;

	TArray<FKey> axisList = { EKeys::MouseX, EKeys::MouseY, EKeys::MouseWheelAxis, EKeys::Gamepad_RightX, EKeys::Gamepad_RightY, 
		EKeys::Gamepad_RightThumbstick, EKeys::Gamepad_LeftX, EKeys::Gamepad_LeftY, EKeys::Gamepad_LeftThumbstick };

	FInputAxisKeyMapping localMap;
	TArray<FInputAxisKeyMapping> fixedArray;
	for ( int32 i = 0; i < _AxisArray.Num(); i++ ) {
		localMap = _AxisArray[i];
		localMap.AxisName = _Axis;
		// Fix and allow to have non axis based from -1 to 1 and from 0 to 1
		if ( i != 0 && localMap.Scale < 0.0f ) {
			if ( axisList.Contains( localMap.Key ) ) {
				localMap.Scale = FMath::Abs( localMap.Scale );
			}
		}
		fixedArray.Add( localMap );
	}

	TArray<FInputAxisKeyMapping> mappings;
	if ( _Input->GetAxisMappings().Num() > 0 ) {
		_Input->GetAxisMappingByName( _Axis, mappings );

		for ( const FInputAxisKeyMapping map : mappings ) {
			_Input->RemoveAxisMapping( map, false );
		}
	}

	for ( const FInputAxisKeyMapping map : fixedArray ) {
		_Input->AddAxisMapping( map, false );

	}

	//_Input->SaveKeyMappings();
	//_Input->ForceRebuildKeymaps();
}

void UCGameSettings::SetKeyBinds() {

	UInputSettings* input{};
	input = const_cast<UInputSettings*>( GetDefault<UInputSettings>() );

	if ( !IsValid( input ) ) {
		UE_LOG( LogTemp, Warning, TEXT( "SetKeyBinds() Class Error" ) );
		return;
	}

	FName cB = "ActionMain";
	SetActionKeysBind( cB, input, { { cB, KActionMain }, { cB, KActionMainAlt } } );
	cB = "ActionSpecial";
	SetActionKeysBind( cB, input, { { cB, KActionSpecial }, { cB, KActionSpecialAlt } } );

	cB = "MoveVertical";
	SetAxisKeysBind( cB, input, { { cB, KMoveForward }, { cB, KMoveForwardAlt },  { cB, KMoveBackward, -1.0f }, { cB, KMoveBackwardAlt, -1.0f } } );
	cB = "MoveHorizontal";
	SetAxisKeysBind( cB, input, { { cB, KMoveRight }, { cB, KMoveRightAlt },  { cB, KMoveLeft, -1.0f }, { cB, KMoveLeftAlt, -1.0f } } );
	cB = "MoveDepth";
	SetAxisKeysBind( cB, input, { { cB, KMoveDepth }, { cB, KMoveDepthAlt }, { cB, KMoveDepthReverse, -1.0f }, { cB, KMoveDepthReverseAlt, -1.0f } } );

	input->SaveKeyMappings();
	input->ForceRebuildKeymaps();
}

#undef LOCTEXT_NAMESPACE
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
#include "Components/LightComponent.h"
#include "Runtime/Engine/Classes/Engine/Light.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundCue.h"
#include "VisyerTypes.generated.h"


#define PLAYLIST_LIMIT 100
#define SONGS_LIMIT 2000

#define SPECTRUM_NUM_NOTES 20
#define AMPLITUDE_NUM_NOTES 20
#define BEAT_NUM_NOTES 3

#define COMPOSER_SPAWN_LIMIT 5
#define COMPOSER_BALLS_PER_SPAWN 100
#define COMPOSER_PITCH_LIMIT 200

UENUM( BlueprintType )
enum class EGameModeList : uint8 {
	eMainMenu		UMETA( DisplayName = "Main Menu" ),
	eMusicPlayer	UMETA( DisplayName = "Music Player" ),
	eMusicComposer	UMETA( DisplayName = "Music Composer" ),
	eGameTypeA		UMETA( DisplayName = "Game Mode A" ),
	eGameTypeB		UMETA( DisplayName = "Game Mode B" )
};

UENUM( BlueprintType )
enum class EGameModeType : uint8 {
	ePlayer		UMETA( DisplayName = "Player" ),
	eComposer	UMETA( DisplayName = "Composer" ),
	eGame		UMETA( DisplayName = "Game" )
};

// Net Radio uses mediaframework and sound source (Can have UE4 effects)
// 

UENUM( BlueprintType )
enum class ETrackFormats : uint8 {
	eOGG		UMETA( DisplayName = "OGG" ), // Custom Audio (Base Supported)(Not in Media Player)
	eWAV		UMETA( DisplayName = "WAV" ), // Custom Audio (Base Supported)
	eMP3		UMETA( DisplayName = "MP3" ), // Custom Audio (Supported with Plugin)
	eFLAC		UMETA( DisplayName = "FLAC" ), // Custom Audio
	eSTREAM		UMETA( DisplayName = "Stream" ), // Media Player
	eSERROR		UMETA( DisplayName = "Error" ) // Error Not Set
};

UENUM( BlueprintType )
enum class EPlaceholderStates : uint8 {
	ePlaceable	UMETA( DisplayName = "Placeable" ),
	eBlocked	UMETA( DisplayName = "Blocked" ),
	eMaximum	UMETA( DisplayName = "Maximum" )
};

UENUM( BlueprintType )
enum class EComposerBlockStates : uint8 {
	eDefault	UMETA( DisplayName = "Default" ),
	eHighlight	UMETA( DisplayName = "Highlight" ),
	eWarning	UMETA( DisplayName = "Blocked" ),
	eInfo		UMETA( DisplayName = "Info" )
};

UENUM( BlueprintType )
enum class EComposerBlockTypes : uint8 {
	eEmpty		UMETA( DisplayName = "Empty" ),
	eSpawn		UMETA( DisplayName = "Spawn" ),
	eSlingshot	UMETA( DisplayName = "Slingshot" ),
	eBumper		UMETA( DisplayName = "Bumper" ),
	eObstacle	UMETA( DisplayName = "Obstacle" )
};

UENUM( BlueprintType )
enum class EComposerInstrumentList : uint8 {
	eInstrumentA		UMETA( DisplayName = "Instrument A" ),
	eInstrumentB		UMETA( DisplayName = "Instrument B" ),
	eInstrumentC		UMETA( DisplayName = "Instrument C" ),
	eInstrumentD		UMETA( DisplayName = "Instrument D" ),
	eInstrumentE		UMETA( DisplayName = "Instrument E" ),
	eInstrumentF		UMETA( DisplayName = "Instrument F" )
};

USTRUCT( BlueprintType )
struct FTrackBatchStruct {
	GENERATED_USTRUCT_BODY()

public:

	// Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Name;

	// URL
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		TArray<FString> URLs;

	FTrackBatchStruct() {
		Name = "Default";
		URLs.Init( "", 1 );
	}

};

UENUM( BlueprintType )
enum class EMusicDeviceList : uint8 { // Devices types
	eSingle		UMETA( DisplayName = "Single" ),
	eListen		UMETA (DisplayName = "Listen" ),
	eCassette	UMETA( DisplayName = "Cassette" ),
	ePurpleRay	UMETA( DisplayName = "PurpleRay" ),
	eRadioLocal	UMETA( DisplayName = "Radio Local" ),
	eRadioNet	UMETA( DisplayName = "Radio Online" )
};

UENUM( BlueprintType )
enum class EDefaultMusicGenresList : uint8 { // Get better user defined tags
	eElectronica,
	eJazz,
	eCinematic,
	eClassical,
	eRock,
	ePop
};

////////////////////
// COMPOSER
////////////////////

USTRUCT( BlueprintType )
struct FComposerTrackStruct {
	GENERATED_USTRUCT_BODY()

public:
	// Song File Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Name;

	// Song URL
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString URL;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString ID;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Title;

	// Artist
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Artist;

	// Genre
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Year;

	// Genre
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Genre;

	// Song Icon
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UTexture2D* Cover;

	// Isn't needed for the media framework, just send the URL
	//UPROPERTY( EditAnywhere, BlueprintReadWrite )
		//USoundWave* Song;

	FComposerTrackStruct() {
		Name = "Default";
		URL = "";
		ID = "Default_Track";
		Title = "Default";
		Artist = "";
		Year = "1999";
		Genre = "Default";
		Cover = nullptr;
	}

};


////////////////////
// MUSIC PLAYER
////////////////////
/*

SongURL//Title//Artist

*/



USTRUCT( BlueprintType )
struct FTrackStruct {
	GENERATED_USTRUCT_BODY()

public:
	// Song File Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Name;

	// Song URL
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString URL;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString ID;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Title;

	// Artist
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Album;

	// Artist
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Artist;

	// Genre
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Year;

	// Genre
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Genre;

	// Song Icon
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UTexture2D* Cover;

	// Song Length, if calculated length is negative then the song is corrupted and won't be played
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		float Length;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		ETrackFormats Format;

	// Isn't needed for the media framework, just send the URL
	//UPROPERTY( EditAnywhere, BlueprintReadWrite )
		//USoundWave* Song;

	FTrackStruct() {
		Name = "Default";
		URL = "";
		ID = "Default_Track";
		Title = "Default";
		Album = "Default_Album";
		Artist = "";
		Year = "1999";
		Genre = "Default";
		Cover = nullptr;
		Length = 0.0f;
		Format = ETrackFormats::eSERROR;
		//Song = nullptr;
	}

};

/*




*/

USTRUCT( BlueprintType )
struct FPlaylistStruct {
	GENERATED_USTRUCT_BODY()

public:

	// PlayList Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Name;

	// PlayList Icon
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UTexture2D* Icon;

	// PlayList Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		EMusicDeviceList DeviceType;

	// PlayList Limit
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		int32 TracksLimit;

	// Artist
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		TArray<FTrackStruct> Tracks;

	// Add the files here on load
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		TArray<FString> TrackFiles;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		TArray<FString> TrackDirectories;

	FPlaylistStruct() {
		Name = "Default";
		Icon = nullptr;
		DeviceType = EMusicDeviceList::eCassette;
		TracksLimit = 0;
		Tracks.Empty();
		TrackFiles.Empty();
		TrackDirectories.Empty();
	}

};


USTRUCT( BlueprintType )
struct FRadioNetStationStruct {
	GENERATED_USTRUCT_BODY()

public:

	// Station Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Name;

	// Station Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Art;

	// Is Internal game radio?
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		bool Internal;

	// Radio URL
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString URL;

	FRadioNetStationStruct() {
		Name = "Default";
		Internal = false;
		URL = "";
	}

};

USTRUCT( BlueprintType )
struct FRadioAdStruct {
	GENERATED_USTRUCT_BODY()

public:

	// Station Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Name;

	// Calculate audio length
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		float Length;

	// Overlap prev song volume
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		bool StartOverlapVolume;

	// Time the ad can start overlap the pre song
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		float StartOverlapLength;

	// Time the next song can start
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		float EndOverlapLength;

	// Overlap post song volume
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		bool EndOverlapVolume;

	// Actual 
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		USoundCue* Audio;

	FRadioAdStruct() {
		Name = "Default";
		Length = 0.0f;
		StartOverlapVolume = false;
		StartOverlapLength = 0.0f;
		EndOverlapLength = 0.0f;
		EndOverlapVolume = false;
		Audio = nullptr;
	}

};

USTRUCT( BlueprintType )
struct FRadioLocalStationStruct {
	GENERATED_USTRUCT_BODY()

public:

	// Station Name ID
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString NameID;

	// Station Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Name;

	// Station Name
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FString Art;

	// Is Internal game radio or from files?
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		bool Internal;

	// Is Internal game radio or from files?
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		int32 Seed;

	// Overlap prev song volume
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		FPlaylistStruct Playlist;

	// Time the ad can start overlap the pre song
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		TArray<FRadioAdStruct> Ads;

	FRadioLocalStationStruct() {
		NameID = "DefaultID01";
		Name = "Default";
		Art = nullptr;
		Internal = false;
		Seed = 8;
		Playlist = {};
		Ads.Empty();
	}

};

namespace EVisyerMatchState
{
	enum Type
	{
		Warmup,
		Playing,
		Won,
		Lost,
	};
}

namespace EVisyerCrosshairDirection
{
	enum Type
	{
		Left=0,
		Right=1,
		Top=2,
		Bottom=3,
		Center=4
	};
}

namespace EVisyerHudPosition
{
	enum Type
	{
		Left=0,
		FrontLeft=1,
		Front=2,
		FrontRight=3,
		Right=4,
		BackRight=5,
		Back=6,
		BackLeft=7,
	};
}

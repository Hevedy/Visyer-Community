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

#include "CGameMusicComp.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CGameInstance.h"
#include "CGameMode.h"
#include "CGameModeGameplay.h"
#include "CGameModeLobby.h"
#include "HEVLibraryMath.h"
#include "HEVLibraryMedia.h"
#include "CGameSettings.h"
#include "Misc/Paths.h"
#include "CMainBlueprintFL.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Core/Public/Async/Async.h"
#include "AudioAnalyzerDecoderFast.h"
#include "DiscordBlueprint.h"


static const FString ResourcesRootDir = UHEVLibraryIO::GameResourcesDir( false );
static const FString ResourcesDir = ResourcesRootDir + "Resources/"; // Public Resources
static const FString ContentDir = UHEVLibraryIO::GameContentDir( false ) + "Media/"; // Protected Resources

// Sets default values for this component's properties
UCGameMusicComp::UCGameMusicComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	RadioSpectrumHz = { 25.f, 30.f, 60.f, 125.f, 250.f, 500.f, 1000.f, 1500.f, 2000.f, 2100.f, 2500.f, 3000.f, 4000.f, 5000.f, 6000.f, 7000.f, 8000.f, 12000.f, 14000.f, 16000.f };
	LocalSpectrumHz = { FVector2D(1.0f, 1.2f) };

	NotesSpectrumDefault.Init( 0.0f, NotesSpectrumNum );
	NotesSpectrum = NotesSpectrumDefault;
	NotesSpectrumForward = NotesSpectrum;
	NotesSpectrumAvg = NotesSpectrumDefault;
	NotesSpectrumAvgForward = NotesSpectrumAvg;

	KeyNoteSpectrum = 0.f;
	KeyNoteSpectrumAvg = 0.f;

	NotesAmplitudeDefault.Init( 0.0f, NotesAmplitudeNum );
	NotesAmplitude = NotesAmplitudeDefault;
	NotesAmplitudeForward = NotesAmplitude;

	KeyNoteAmplitude = 0.f;
	KeyNoteAmplitudeForward = KeyNoteAmplitude;

	KeyPitch = 0.f;
	KeyPitchForward = KeyPitch;

	BeatsStatusDefault.Init( false, 3 );
	BeatsAmplitudeDefault.Init( 0.0f, 3 );
	BeatsPMDefault.Init( 0, 3 );
	BeatsStatus = BeatsStatusDefault;
	BeatsAmplitude = BeatsAmplitudeDefault;
	BeatsPM = BeatsPMDefault;
	BeatsStatusForward = BeatsStatus;
	BeatsAmplitudeForward = BeatsAmplitude;
	BeatsPMForward = BeatsPM;
	Beats = 0;
	BeatsForward = Beats;

	FrequencyVisualScalesDefault.Init( 1.0f, 20 );
	FrequencyVisualScales = FrequencyVisualScalesDefault;

	FrequencyVisualMasterScale = 1.f;

	FrequencyVibrationScalesDefault.Init( 0.0f, 3 );
	FrequencyVibrationScales = FrequencyVibrationScalesDefault;

	FrequencyVibrationMasterScale = 0.0f;

	// ...
}


// Called when the game starts
void UCGameMusicComp::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}


// Called every frame
void UCGameMusicComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if ( !Initialized ) {
		return;
	}
	UpdateParams();

	// ...
}

bool UCGameMusicComp::GetPlaylistThreadData( int32& _Loaded, int32& _Total, float& _Percentage ) {
	_Loaded = ThreadLoadedPlaylistsNum;
	_Total = ThreadLoadingPlaylistsNum;
	_Percentage = ThreadLoadedPlaylistsNum / ThreadLoadingPlaylistsNum;
	return ThreadLoadingPlaylists;
}

bool UCGameMusicComp::GetTrackThreadData( int32& _Loaded, int32& _Total, float& _Percentage ) {
	_Loaded = ThreadLoadedTracksNum;
	_Total = ThreadLoadingTracksNum;
	_Percentage = ThreadLoadedTracksNum / ThreadLoadingTracksNum;
	return ThreadLoadingTracks;
}


void UCGameMusicComp::DefaultSettings() {

}

bool UCGameMusicComp::Setup_Implementation( const bool _InGame ) {
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			GameModeType = lGI->GameModeType;
			if ( _InGame ) {
				InGame = _InGame;
			} else {
				if ( GameModeType != EGameModeList::eMainMenu && GameModeType != EGameModeList::eMusicPlayer ) {
					InGame = true;
				} else {
					InGame = false;
				}
			}
			if ( !lGI->MusicPlayerFilesSet ) {
				ReadFiles();

			} else {
				if ( ReadFilesSimple() ) {

				}
			}

		}
		return true;
	} else {
		return false;
	}
}

// ====================
// FILES READ AND PARSE
// ====================


TArray<FString> UCGameMusicComp::GetTierBlackListedResources() {
	TArray<FString> blacklistedResources = TArray<FString>{}; //= { "", "" }; //"", "" 
	switch ( UCMainBlueprintFL::GetGameTier() ) {
		case EGameTierTypesList::ETier0:
			blacklistedResources = { 
				""
			};
			break;
		case EGameTierTypesList::ETier1:
			blacklistedResources = TArray<FString>{};
			break;
		case EGameTierTypesList::ETier2:
			blacklistedResources = TArray<FString>{};
			break;
	}
	return blacklistedResources;
}

bool UCGameMusicComp::ValidateGetTrackURL( const FString& _TrackURL, FString& _TrackURLFixed ) {
	FString trackFile = _TrackURL;
	if ( trackFile.StartsWith( "[R]/" ) ) { // Resources Folder
		trackFile = trackFile.Mid( 4, trackFile.Len() - 4 );
		trackFile = *ResourcesDir + trackFile;
		trackFile = FPaths::ConvertRelativePathToFull( trackFile );
	} else if ( trackFile.StartsWith( "[C]/" ) ) { // Content Folder
			trackFile = trackFile.Mid( 4, trackFile.Len() - 4 );
			trackFile = *ResourcesRootDir + trackFile;
			trackFile = FPaths::ConvertRelativePathToFull( trackFile );
	} else if ( trackFile.StartsWith( "[P]/" ) ) { // Media Content folder, Private internal
		if ( UCMainBlueprintFL::GetGameTierIsStandard() ) {
			trackFile = trackFile.Mid( 4, trackFile.Len() - 4 );
			trackFile = *ContentDir + trackFile;
			trackFile = FPaths::ConvertRelativePathToFull( trackFile );
		} else {
			trackFile = "";
		}
	}
	FPaths::NormalizeDirectoryName( trackFile );
	if ( UHEVLibraryIO::FileExistsIsValidExt( trackFile, { ".ogg", ".wav", ".mp3", ".flac" } ) ) {
		_TrackURLFixed = trackFile;
		return true;
	} else {
		_TrackURLFixed = ""; //_TrackURL
		return false;
	}
}

bool UCGameMusicComp::ValidateGetRadioURL( const FString& _WebURL, FString& _WebURLFixed ) {
	FString webURL = _WebURL;
	if ( webURL != "" || webURL != " " || webURL != "youtu" || webURL != ".be" || webURL != "vimeo" ) {
		if ( webURL.Contains( "https://" ) || webURL.Contains( "http://" ) ) {
			if ( webURL.Contains( ".fm" ) || webURL.Contains( ".radio" ) || webURL.Contains( "radio." ) || webURL.Contains( "stream" ) || webURL.Contains( ".com" ) ) {
				_WebURLFixed = webURL;
				return true;
			}
		}
	}
	_WebURLFixed = ""; //_WebURL
	return false;
}

bool UCGameMusicComp::PushSingleData( const FString& _URL ) {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::PushSingleData()" ) );
	FString url = "";
	if ( ValidateGetTrackURL( _URL, url ) ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI == nullptr ) {
			return false;
		}
		lGI->GameSettings->TrackPlayerSingle = url;
		lGI->GameSettings->WriteString( "TrackPlayer", "Single", lGI->GameSettings->TrackPlayerSingle, ESettingFilesList::eUserSettings );
		return true;
	} else {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::PushSingleData() ERROR" ) );
		return false;
	}
}

bool UCGameMusicComp::TrySetSingle( const FString& _URL ) {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::TrySetSingle()" ) );
	FString url = AudioStreamChannel;
	if ( ValidateGetTrackURL( _URL, url ) ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI == nullptr ) {
			return false;
		}
		lGI->GameSettings->TrackPlayerSingle = url;
		UE_LOG( LogTemp, Warning, TEXT( "TrySetSingle() TRACK SET" ) );
		SetDevice( EMusicDeviceList::eSingle );
		//SetPlaylistIndex( 0 );
		SetPlaylistIndex( FThreadLoadedBP{}, 0, 0, true );
		lGI->GameSettings->WriteString( "TrackPlayer", "Single", lGI->GameSettings->TrackPlayerSingle, ESettingFilesList::eUserSettings );
		return true;
	} else {
		UE_LOG( LogTemp, Warning, TEXT( "TrySetSingle() ERROR" ) );
		return false;
	}
}

bool UCGameMusicComp::TrySetListen() {
	UE_LOG(LogTemp, Warning, TEXT("UCGameMusicComp::TrySetListen()"));

	if (AudioStreamChannel != "") {
		UCGameInstance* lGI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
		if (lGI == nullptr) {
			return false;
		}
		UE_LOG(LogTemp, Warning, TEXT("TrySetListen() STREAM SET"));
		SetDevice(EMusicDeviceList::eListen);
		SetPlaylistIndex(FThreadLoadedBP{}, 0, 0, true);
		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("TrySetListen() ERROR"));
		return false;
	}
}

FString UCGameMusicComp::GetDeviceName() {
	FString name = "";
	switch ( Device ) {
		case EMusicDeviceList::eSingle:
			name = "Single";
			break;
		case EMusicDeviceList::eListen:
			name = "Listening";
			break;
		case EMusicDeviceList::eCassette:
			name = "Cassette";
			break;
		case EMusicDeviceList::ePurpleRay:
			name = "PurpleRay";
			break;
		case EMusicDeviceList::eRadioLocal:
			name = "Radio LC";
			break;
		case EMusicDeviceList::eRadioNet:
			name = "Radio";
			break;
		default:
			break;
	}
	return name;
}


void UCGameMusicComp::GetSetInstanceDevicePlaylist() {

	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			// Get Set Device

			switch ( lGI->GameSettings->TrackPlayerAutoPlayIndex ) {
				case 0:
					TrackLoopPlay = false;
					PlaylistAutoPlay = false;
					PlaylistRandomOrder = false;
					break;
				case 1:
					TrackLoopPlay = true;
					PlaylistAutoPlay = false;
					PlaylistRandomOrder = false;
					break;
				case 2:
					TrackLoopPlay = false;
					PlaylistAutoPlay = true;
					PlaylistRandomOrder = false;
					break;
				case 3:
					TrackLoopPlay = false;
					PlaylistAutoPlay = false;
					PlaylistRandomOrder = true;
					break;
				default:
					TrackLoopPlay = false;
					PlaylistAutoPlay = false;
					PlaylistRandomOrder = false;
					break;
			}

			switch ( lGI->GameSettings->TrackPlayerDeviceIndex ) {
				case 0:
					Device = EMusicDeviceList::eSingle;
					break;
				case 1:
					Device = EMusicDeviceList::eListen;
					break;
				case 2:
					Device = EMusicDeviceList::eCassette;
					break;
				case 3:
					Device = EMusicDeviceList::ePurpleRay;
					break;
				case 4:
					Device = EMusicDeviceList::eRadioLocal;
					break;
				case 5:
					Device = EMusicDeviceList::eRadioNet;
					break;
				default:
					Device = EMusicDeviceList::eCassette;
					break;
			}
			switch ( Device ) {
				case EMusicDeviceList::eSingle:
					PlaylistIndex = 0;
					break;
				case EMusicDeviceList::eListen:
					PlaylistIndex = 0;
					break;
				case EMusicDeviceList::eCassette:
					lGI->MusicPlayerCassetteIndex = lGI->GameSettings->TrackPlayerPlaylistIndex;
					PlaylistIndex = lGI->MusicPlayerCassetteIndex;
					break;
				case EMusicDeviceList::ePurpleRay:
					lGI->MusicPlayerPurpleRayIndex = lGI->GameSettings->TrackPlayerPlaylistIndex;
					PlaylistIndex = lGI->MusicPlayerPurpleRayIndex;
					break;
				case EMusicDeviceList::eRadioLocal:
					lGI->MusicPlayerLocalRadioStationIndex = lGI->GameSettings->TrackPlayerPlaylistIndex;
					PlaylistIndex = lGI->MusicPlayerLocalRadioStationIndex;
					break;
				case EMusicDeviceList::eRadioNet:
					lGI->MusicPlayerNetRadioStationIndex = lGI->GameSettings->TrackPlayerPlaylistIndex;
					PlaylistIndex = lGI->MusicPlayerNetRadioStationIndex;
					break;
			}
		}
	}
}

void UCGameMusicComp::SetInstanceDevicePlaylist() {
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			// Set Device

			if ( TrackLoopPlay ) {
				lGI->GameSettings->TrackPlayerAutoPlayIndex = 1;
			} else if ( PlaylistAutoPlay ) {
				lGI->GameSettings->TrackPlayerAutoPlayIndex = 2;
			} else if ( PlaylistRandomOrder ) {
				lGI->GameSettings->TrackPlayerAutoPlayIndex = 3;
			} else {
				lGI->GameSettings->TrackPlayerAutoPlayIndex = 0;
			}

			switch ( Device ) {
				case EMusicDeviceList::eSingle:
					lGI->GameSettings->TrackPlayerPlaylistIndex = 0;
					lGI->GameSettings->TrackPlayerDeviceIndex = 0;
					break;
				case EMusicDeviceList::eListen:
					lGI->GameSettings->TrackPlayerPlaylistIndex = 0;
					lGI->GameSettings->TrackPlayerDeviceIndex = 0;
					break;
				case EMusicDeviceList::eCassette:
					lGI->MusicPlayerCassetteIndex = PlaylistIndex;
					lGI->GameSettings->TrackPlayerPlaylistIndex = lGI->MusicPlayerCassetteIndex;
					lGI->GameSettings->TrackPlayerDeviceIndex = 1;
					break;
				case EMusicDeviceList::ePurpleRay:
					lGI->MusicPlayerPurpleRayIndex = PlaylistIndex;
					lGI->GameSettings->TrackPlayerPlaylistIndex = lGI->MusicPlayerPurpleRayIndex;
					lGI->GameSettings->TrackPlayerDeviceIndex = 2;
					break;
				case EMusicDeviceList::eRadioLocal:
					lGI->MusicPlayerLocalRadioStationIndex = PlaylistIndex;
					lGI->GameSettings->TrackPlayerPlaylistIndex = lGI->MusicPlayerLocalRadioStationIndex;
					lGI->GameSettings->TrackPlayerDeviceIndex = 3;
					break;
				case EMusicDeviceList::eRadioNet:
					lGI->MusicPlayerNetRadioStationIndex = PlaylistIndex;
					lGI->GameSettings->TrackPlayerPlaylistIndex = lGI->MusicPlayerNetRadioStationIndex;
					//PlaylistIndex = 0;
					lGI->GameSettings->TrackPlayerDeviceIndex = 4;
					break;
			}
		}
	}
}

bool UCGameMusicComp::ReadFiles() {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::ReadFiles()" ) );
	OnFilesReadStart.Broadcast();

	//TArray<FString> blacklistedResources = GetTierBlackListedResources();

	if ( UHEVLibrarySettings::ResourcesDirectoryValidate( true, GetTierBlackListedResources() ) ) {
		const int32 devicesPlaylistLimit = 100;

		GetSetInstanceDevicePlaylist();

		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			FrequencyVisualMasterScale = lGI->GameSettings->HzVisualMaster;
			FrequencyVisualScales = lGI->GameSettings->HzVisualValues;

			FrequencyVibrationMasterScale = lGI->GameSettings->ForceFeedbackShakeIntensity;
			FrequencyVibrationScales = lGI->GameSettings->ForceFeedbackShakeIntensitiesHz;
		}

		// Load and Set the base

		ReloadDevicePlaylists( Device ); //TODO Clear the reader and add dirs and files only

		if ( !ThreadLoadingTracks ) {
			ThreadLoadingTracks = true;
			auto Callback = [&]( int Param1 ) {
				//Do something when finished
				this->ThreadLoadingTracks = false;
				if ( !this->ThreadLoadingPlaylists && !this->ThreadLoadingTracks ) {
					UE_LOG( LogTemp, Warning, TEXT( "ThreadLoadingPlaylists %d" ), this->ThreadLoadingPlaylists ? 1 : 0 );
					UE_LOG( LogTemp, Warning, TEXT( "ThreadLoadingTracks %d" ), this->ThreadLoadingTracks ? 1 : 0 );
					if ( this->GetWorld() != nullptr && this->GetWorld()->GetGameInstance() != nullptr ) {
						UCGameInstance* lGI = Cast<UCGameInstance>( this->GetWorld()->GetGameInstance() );
						if ( lGI != nullptr ) {
							lGI->MusicPlayerFilesSet = true;
						}
					}
					this->InitializeAudioPlayer();
					this->Initialized = true;
					this->OnInitialized.Broadcast();
					UE_LOG( LogTemp, Warning, TEXT( "Completed to read resources." ) );
					this->OnFilesReadEnd.Broadcast();
				}
			};

			AsyncTask( ENamedThreads::AnyThread, [&, Callback]() {
				this->ReloadPlaylistTracks( this->Device, this->PlaylistIndex );

				AsyncTask( ENamedThreads::GameThread, [&, Callback]() {
					Callback( 0 );
				} );
			} );
		}

		UE_LOG( LogTemp, Warning, TEXT( "Resources Called." ) );

		return true;
	} else {
		UE_LOG( LogTemp, Warning, TEXT( "Failed to read resources." ) );
		return false;
	}
}

bool UCGameMusicComp::ReadFilesSimple() {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::ReadFilesSimple()" ) );
	OnFilesReadStart.Broadcast();

	//TArray<FString> blacklistedResources = GetTierBlackListedResources();

	const int32 devicesPlaylistLimit = 100;

	GetSetInstanceDevicePlaylist();

	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI != nullptr ) {
		FrequencyVisualMasterScale = lGI->GameSettings->HzVisualMaster;
		FrequencyVisualScales = lGI->GameSettings->HzVisualValues;

		FrequencyVibrationMasterScale = lGI->GameSettings->ForceFeedbackShakeIntensity;
		FrequencyVibrationScales = lGI->GameSettings->ForceFeedbackShakeIntensitiesHz;
	}

	InitializeAudioPlayer();
	Initialized = true;
	OnInitialized.Broadcast();
	UE_LOG( LogTemp, Warning, TEXT( "Completed to read resources." ) );

	OnFilesReadEnd.Broadcast();


	UE_LOG( LogTemp, Warning, TEXT( "Resources Called." ) );

	return true;
}

void UCGameMusicComp::ReloadDevicePlaylists( const EMusicDeviceList _Device ) {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::ReloadDevicePlaylists()" ) );
	// Per build locked resources
	TArray<FString> blacklistedResources = GetTierBlackListedResources();
	const int32 devicesPlaylistLimit = 100;
	TArray<FString> cfgList;

	FTrackStruct singleTrack;
	TArray<FPlaylistStruct> cassetteList;
	TArray<FPlaylistStruct> purleRayList;
	TArray<FRadioLocalStationStruct> radioLocalList;
	TArray<FRadioNetStationStruct> radioOnlineList;

	if ( GetWorld() == nullptr && GetWorld()->GetGameInstance() == nullptr ) {
		return;
	}
	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI == nullptr ) {
		return;
	}

	switch ( _Device ) {
		case EMusicDeviceList::eSingle:
			// Read directly the settings file from user
			// Do nothing here at all
			singleTrack = FTrackStruct();
			singleTrack.URL = lGI->GameSettings->TrackPlayerSingle;
			singleTrack.Format = ETrackFormats::eSERROR;
			lGI->MP_SingleTrack = singleTrack;
			break;
		case EMusicDeviceList::eListen:
			// Read directly the settings file from user
			// Do nothing here at all
			singleTrack = FTrackStruct();
			singleTrack.URL = "";
			singleTrack.Format = ETrackFormats::eSERROR;
			break;
		case EMusicDeviceList::eCassette:
			if ( UHEVLibraryIO::GetDirectoryFiles( cfgList, ResourcesRootDir + "Cassettes/", ".vpl" ) ) {
				for ( const FString& cfgFile : cfgList ) { // Individual playlist cfg files
					FPlaylistStruct playlist;
					playlist.Name = FPaths::GetBaseFilename( cfgFile );
					playlist.TracksLimit = 12 * 2;
					playlist.DeviceType = EMusicDeviceList::eCassette;
					// TODO Icon, search in future in the resources folder for this.
					int32 songsCount = 0;
					TArray<FString> tracksListUrl;
					if ( UHEVLibrarySettings::ReadTextLines( tracksListUrl, cfgFile ) ) { // Can read the given file
						tracksListUrl = UHEVLibraryIOParser::DeleteArrayDuplicates( tracksListUrl, {}, { ".ogg", ".wav", ".mp3", ".flac" } );
						for ( const FString& trackFile : tracksListUrl ) {
							FString trackURL;
							if ( ValidateGetTrackURL( trackFile, trackURL ) ) {
								if ( UHEVLibraryIO::FileOrDirectory( trackURL ) ) {
									playlist.TrackFiles.Add( trackURL );
								} else {
									playlist.TrackDirectories.Add( trackURL );
								}
								songsCount++; // Probably only count the files in the future
								if ( songsCount > playlist.TracksLimit ) {
									break;
								}
							} else {
								continue; //Next
							}
						}
						if ( playlist.TrackFiles.Num() != 0 ) { // Add playlist to the list
							cassetteList.Add( playlist );
						}
					}
					if ( cassetteList.Num() > devicesPlaylistLimit ) {
						break;
					}
				}
			}
			lGI->MP_Cassettes = cassetteList;
			break;
		case EMusicDeviceList::ePurpleRay:
			if ( UHEVLibraryIO::GetDirectoryFiles( cfgList, ResourcesRootDir + "PurpleRays/", ".vpl" ) ) {
				for ( const FString& cfgFile : cfgList ) { // Individual playlist cfg files
					FPlaylistStruct playlist;
					playlist.Name = FPaths::GetBaseFilename( cfgFile );
					playlist.TracksLimit = 500;//UCMainBlueprintFL::SongsLimit();
					playlist.DeviceType = EMusicDeviceList::ePurpleRay;
					// TODO Icon, search in future in the resources folder for this.
					//file
					int32 songsCount = 0;
					TArray<FString> tracksListUrl;
					if ( UHEVLibrarySettings::ReadTextLines( tracksListUrl, cfgFile ) ) { // Can read the given file
						tracksListUrl = UHEVLibraryIOParser::DeleteArrayDuplicates( tracksListUrl, {}, { ".ogg", ".wav", ".mp3", ".flac" } );
						for ( const FString& trackFile : tracksListUrl ) {
							FString trackURL;
							if ( ValidateGetTrackURL( trackFile, trackURL ) ) {
								if ( UHEVLibraryIO::FileOrDirectory( trackURL ) ) {
									playlist.TrackFiles.Add( trackURL );
								} else {
									playlist.TrackDirectories.Add( trackURL );
								}
								//playlist.TrackFiles.Add( trackURL );
								songsCount++;
								if ( songsCount > playlist.TracksLimit ) {
									break;
								}
							} else {
								continue; //Next
							}
						}

						if ( playlist.TrackFiles.Num() != 0 ) {
							purleRayList.Add( playlist );
						}

					}
					if ( purleRayList.Num() > devicesPlaylistLimit ) {
						break;
					}

				}
			}
			lGI->MP_PurpleRays = purleRayList;
			break;
		case EMusicDeviceList::eRadioLocal:
			if ( UHEVLibraryIO::GetDirectoryFiles( cfgList, ResourcesRootDir + "Radios/Local/", ".vpl" ) ) {
				for ( const FString& cfgFile : cfgList ) { // Individual playlist cfg files
					FRadioLocalStationStruct radioLocal;
					radioLocal.Name = FPaths::GetBaseFilename( cfgFile );
					radioLocal.NameID = radioLocal.Name;
					radioLocal.Internal = false;
					radioLocal.Seed = FMath::RandRange( 1000, 9999 );
					radioLocal.Playlist.Name = radioLocal.Name;
					radioLocal.Playlist.TracksLimit = 500;//UCMainBlueprintFL::SongsLimit();
					radioLocal.Playlist.DeviceType = EMusicDeviceList::eRadioLocal;

					TArray<FRadioAdStruct> radioAds;
					FRadioAdStruct radioAd;
					// TODO Icon, search in future in the resources folder for this.
					//file
					int32 songsCount = 0;
					TArray<FString> tracksListUrl;
					if ( UHEVLibrarySettings::ReadTextLines( tracksListUrl, cfgFile ) ) { // Can read the given file
						tracksListUrl = UHEVLibraryIOParser::DeleteArrayDuplicates( tracksListUrl, {}, { ".ogg", ".wav", ".mp3", ".flac" } );
						for ( const FString& trackFile : tracksListUrl ) {
							FString trackURL;
							if ( ValidateGetTrackURL( trackFile, trackURL ) ) {
								if ( UHEVLibraryIO::FileOrDirectory( trackURL ) ) {
									radioLocal.Playlist.TrackFiles.Add( trackURL );
								} else {
									radioLocal.Playlist.TrackDirectories.Add( trackURL );
								}
								//radioLocal.Playlist.TrackFiles.Add( trackURL );
								songsCount++;
								if ( songsCount > radioLocal.Playlist.TracksLimit ) {
									break;
								}
							} else {
								continue; //Next
							}
						}

						if ( radioLocal.Playlist.TrackFiles.Num() != 0 ) {
							radioLocalList.Add( radioLocal );
						}
					}
					if ( radioLocalList.Num() > devicesPlaylistLimit ) {
						break;
					}
				}
			}

			lGI->MP_RadioLocalStations = radioLocalList;
			break;
		case EMusicDeviceList::eRadioNet:
			if ( UHEVLibraryIO::GetDirectoryFiles( cfgList, ResourcesRootDir + "Radios/Online/", ".vpl" ) ) {
				for ( const FString& cfgFile : cfgList ) { // Individual playlist cfg files
					FRadioNetStationStruct radio;
					radio.Name = FPaths::GetBaseFilename( cfgFile );
					radio.Internal = false;
					TArray<FString> radioListUrl;
					if ( UHEVLibrarySettings::ReadTextLines( radioListUrl, cfgFile, true, true ) ) { // Can read the given file
						if ( radioListUrl.Num() > 0 ) {
							//Validate the online file
							//radio.URL = radioListUrl[0];
							if ( ValidateGetRadioURL( radioListUrl[0], radio.URL ) ) {
								radioOnlineList.Add( radio ); //Here
							}
						}
					}
					if ( radioOnlineList.Num() > devicesPlaylistLimit ) {
						break;
					}
				}
			}
			lGI->MP_RadioNetStations = radioOnlineList;
			break;
		default:
			break;
	}
	cfgList.Empty();
	cassetteList.Empty();
	purleRayList.Empty();
	radioLocalList.Empty();
	radioOnlineList.Empty();
}

void UCGameMusicComp::ReloadPlaylistTracks( const EMusicDeviceList _Device, const int32 _PlaylistIndex, const bool _ForceReload ) {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::ReloadPlaylistTracks()" ) );
	// Per build locked resources
	TArray<FString> blacklistedResources = GetTierBlackListedResources();
	TArray<FString> filesList;
	TArray<FString> filesRawList;
	TArray<FTrackStruct> sourceTracks;
	TArray<FTrackStruct> fixedTracks;
 
	if ( GetWorld() == nullptr && GetWorld()->GetGameInstance() == nullptr ) {
		return;
	}
	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI == nullptr ) {
		return;
	}
	if ( GetDevicePlaylistNum( _Device ) <= 0 ) {
		return;
	}

	int32 lastPlaylistIndex = FMath::Min( GetDevicePlaylistNum( _Device ) - 1, _PlaylistIndex);
	//UE_LOG( LogTemp, Warning, TEXT( "PLAYLIST INDEX %d" ), lastPlaylistIndex );

	if ( lastPlaylistIndex < 0 ) {
		return;
	}

	int32 playlistTrackLimit = 0;
	int32 playlistTrackCount = 0;
	//int32 lastTrackIndex = GetPlaylistTracksNum( _Device, _PlaylistIndex );

	switch ( _Device ) {
		case EMusicDeviceList::eSingle:
			{ // Let the var execute
				playlistTrackLimit = 1;
				// Combine Folders and Files
				lGI->MP_SingleTrack.URL = lGI->GameSettings->TrackPlayerSingle;
				FString trackFixedURL;
				if ( ValidateGetTrackURL( lGI->MP_SingleTrack.URL, trackFixedURL ) ) { // Validation One Make Sure the Files Exist
					FTrackStruct curTrack;
					curTrack.URL = trackFixedURL;
					sourceTracks.Add( curTrack );
				}
				if ( sourceTracks.Num() > 0 ) {
					fixedTracks = GetTracks( sourceTracks, playlistTrackLimit, true ); // Validate tracks and set it
					if ( fixedTracks.Num() > 0 ) {
						lGI->MP_SingleTrack = fixedTracks[0];
					}
				}
			}
			break;
		case EMusicDeviceList::eListen:
			{ // Let the var execute
				playlistTrackLimit = 1;
				// Validation One Make Sure the Files Exist
				FTrackStruct curTrack;
				curTrack.URL = "";
				sourceTracks.Add(curTrack);
				if (sourceTracks.Num() > 0) {
					fixedTracks = sourceTracks; // Validate tracks and set it
					if (fixedTracks.Num() > 0) {
						//fixedTracks[0];
					}
				}
			}
			break;
		case EMusicDeviceList::eCassette:
			if ( lGI->MP_Cassettes[lastPlaylistIndex].Tracks.Num() > 0 && !_ForceReload ) {
				return;
			}
			playlistTrackLimit = lGI->MP_Cassettes[lastPlaylistIndex].TracksLimit;
			filesRawList = lGI->MP_Cassettes[lastPlaylistIndex].TrackFiles; //Possible error at the new function
			filesRawList.Append( lGI->MP_Cassettes[lastPlaylistIndex].TrackDirectories );
			//for ( const FString& track : lGI->MP_Cassettes[lastPlaylistIndex].TrackFiles ) {
			if ( UHEVLibraryIO::GetFilesFromRawPaths( filesRawList, filesList, { ".ogg", ".wav", ".mp3", ".flac" }, true ) ) {
			}
			for ( const FString& srcURL : filesList ) {
				FString trackFixedURL;
				if ( ValidateGetTrackURL( srcURL, trackFixedURL ) ) { // Validation One Make Sure the Files Exist
					FTrackStruct curTrack;
					curTrack.URL = trackFixedURL;
					sourceTracks.Add( curTrack );
					playlistTrackCount++;
					if ( playlistTrackCount >= playlistTrackLimit ) {
						break;
					}
				}
			}
			if ( sourceTracks.Num() > 0 ) {
				fixedTracks = GetTracks( sourceTracks, playlistTrackLimit, true ); // Validate tracks and set it
				if ( fixedTracks.Num() > 0 ) {
					lGI->MP_Cassettes[lastPlaylistIndex].Tracks = fixedTracks;
				}
			}
			//}
			//GetTracks( songList, lGI->MP_Cassettes[lastPlaylistIndex].TracksLimit );
			break;
		case EMusicDeviceList::ePurpleRay:
			if ( lGI->MP_PurpleRays[lastPlaylistIndex].Tracks.Num() > 0 && !_ForceReload ) {
				return;
			}
			playlistTrackLimit = lGI->MP_PurpleRays[lastPlaylistIndex].TracksLimit;
			filesRawList = lGI->MP_PurpleRays[lastPlaylistIndex].TrackFiles; //Possible error at the new function
			filesRawList.Append( lGI->MP_PurpleRays[lastPlaylistIndex].TrackDirectories );
			//for ( const FString& track : lGI->MP_Cassettes[lastPlaylistIndex].TrackFiles ) {
			if ( UHEVLibraryIO::GetFilesFromRawPaths( filesRawList, filesList, { ".ogg", ".wav", ".mp3", ".flac" }, true ) ) {
			}
			for ( const FString& srcURL : filesList ) {
				FString trackFixedURL;
				if ( ValidateGetTrackURL( srcURL, trackFixedURL ) ) { // Validation One Make Sure the Files Exist
					FTrackStruct curTrack;
					curTrack.URL = trackFixedURL;
					sourceTracks.Add( curTrack );
					playlistTrackCount++;
					if ( playlistTrackCount >= playlistTrackLimit ) {
						break;
					}
				}
			}
			if ( sourceTracks.Num() > 0 ) {
				fixedTracks = GetTracks( sourceTracks, playlistTrackLimit, true ); // Validate tracks and set it
				if ( fixedTracks.Num() > 0 ) {
					lGI->MP_PurpleRays[lastPlaylistIndex].Tracks = fixedTracks;
				}
			}
			break;
		case EMusicDeviceList::eRadioLocal:
			if ( lGI->MP_RadioLocalStations[lastPlaylistIndex].Playlist.Tracks.Num() > 0 && !_ForceReload ) {
				return;
			}
			playlistTrackLimit = lGI->MP_RadioLocalStations[lastPlaylistIndex].Playlist.TracksLimit;
			filesRawList = lGI->MP_RadioLocalStations[lastPlaylistIndex].Playlist.TrackFiles; //Possible error at the new function
			filesRawList.Append( lGI->MP_RadioLocalStations[lastPlaylistIndex].Playlist.TrackDirectories );
			//for ( const FString& track : lGI->MP_Cassettes[lastPlaylistIndex].TrackFiles ) {
			if ( UHEVLibraryIO::GetFilesFromRawPaths( filesRawList, filesList, { ".ogg", ".wav", ".mp3", ".flac" }, true ) ) {
			}
			for ( const FString& srcURL : filesList ) {
				FString trackFixedURL;
				if ( ValidateGetTrackURL( srcURL, trackFixedURL ) ) { // Validation One Make Sure the Files Exist
					FTrackStruct curTrack;
					curTrack.URL = trackFixedURL;
					sourceTracks.Add( curTrack );
					playlistTrackCount++;
					if ( playlistTrackCount >= playlistTrackLimit ) {
						break;
					}
				}
			}
			if ( sourceTracks.Num() > 0 ) {
				fixedTracks = GetTracks( sourceTracks, playlistTrackLimit, true ); // Validate tracks and set it
				if ( fixedTracks.Num() > 0 ) {
					lGI->MP_RadioLocalStations[lastPlaylistIndex].Playlist.Tracks = fixedTracks;
				}
			}
			break;
		case EMusicDeviceList::eRadioNet:
			if ( lGI->MP_RadioNetStations[lastPlaylistIndex].URL == "" && !_ForceReload ) {
				return;
			}
			{ // Let the var execute
				playlistTrackLimit = 1;
				FString radioFixedURL;
				if ( !ValidateGetRadioURL( lGI->MP_RadioNetStations[lastPlaylistIndex].URL, radioFixedURL ) ) { // Validation One Make Sure the Files Exist
					break;
				}
				FRadioNetStationStruct radio = lGI->MP_RadioNetStations[lastPlaylistIndex];
				radio.URL = radioFixedURL;
				lGI->MP_RadioNetStations[lastPlaylistIndex] = radio;
			}
			break;
		default:
			break;
	}
}

void UCGameMusicComp::ReloadDevice( FThreadLoadedDevice _OnDeviceComplete, const EMusicDeviceList _Device ) {
	if ( ThreadLoading ) {
		UE_LOG( LogTemp, Warning, TEXT( "Thread In Use" ));
		return;
	}
	ThreadLoading = true;
	OnThread.Broadcast( true );

	ReloadDevicePlaylists( _Device );
	ThreadLoading = false;
	OnThread.Broadcast( false );
	_OnDeviceComplete.Execute();
}

void UCGameMusicComp::ReloadPlaylist( FThreadLoadedPlaylist _OnPlaylistComplete, const EMusicDeviceList _Device, const int32 _PlaylistIndex ) {
	if ( ThreadLoading ) {
		UE_LOG( LogTemp, Warning, TEXT( "Thread In Use" ) );
		return;
	}
	ThreadLoading = true;
	OnThread.Broadcast( true );
	//ReloadPlaylistTracks( _Device, _PlaylistIndex, true );


	AsyncTask( ENamedThreads::AnyThread, [&, _OnPlaylistComplete, _Device, _PlaylistIndex]() {
		ReloadPlaylistTracks( _Device, _PlaylistIndex, true );

		AsyncTask( ENamedThreads::GameThread, [&, _OnPlaylistComplete, _Device, _PlaylistIndex]() {
			ThreadLoading = false;
			OnThread.Broadcast( false );
			_OnPlaylistComplete.Execute();
		} );
	} );
}



// ====================
// MAIN THREAD CORE
// ====================

void UCGameMusicComp::InitializeAudioPlayer() {
	MediaPlayer = NewObject<UMediaPlayer>( this, UMediaPlayer::StaticClass() );
	AudioPlayer = NewObject<UAudioAnalyzerManager>( this );
	AudioPlayerForward = NewObject<UAudioAnalyzerManager>( this );
	UE_LOG(LogTemp, Warning, TEXT("InitializeAudioPlayer() - Unloaders"));
	AudioPlayer->UnloadCapturerAudio();
	AudioPlayer->UnloadLoopbackAudio(); // System for window stream
	AudioPlayer->UnloadPlayerAudio();
	// Audio helper for in game systems
	AudioPlayerForward->UnloadCapturerAudio();
	AudioPlayerForward->UnloadPlayerAudio();
	UE_LOG(LogTemp, Warning, TEXT("InitializeAudioPlayer() - Unloaders Finish"));

	if ( !InGame ) {
		ACGameMode* lGM = Cast<ACGameMode>( GetWorld()->GetAuthGameMode() );
		if ( lGM != nullptr ) {
			if ( lGM->WorldParamsClass->IsChildOf( UMaterialParameterCollection::StaticClass() ) && lGM->WorldParamsClass.GetDefaultObject() ) {
				WorldParams = NewObject<UMaterialParameterCollection>( this, lGM->WorldParamsClass );
			}
			MediaPlayer = lGM->MediaSoundComponent->GetMediaPlayer();
			lGM->MediaSoundComponent->SetMediaPlayer( MediaPlayer );
			MediaSoundComponent = lGM->MediaSoundComponent;


			MediaSoundComponent->SetSpectralAnalysisSettings( RadioSpectrumHz );
			MediaSoundComponent->SetEnableSpectralAnalysis( true );
		}
	}
	TArray<FString> outputDevices;
	AudioPlayer->GetOutputAudioDevices( outputDevices );
	AudioPlayer->SetDefaultDevicePlayerAudio( outputDevices[0] );
	AudioPlayer->SetDefaultDeviceLoopbackAudio( outputDevices[0] );
	AudioPlayerForward->SetPlaybackVolume( 0.0f );

	UE_LOG(LogTemp, Warning, TEXT("InitializeAudioPlayer() - Device: %s"), *outputDevices[0]);

	AudioStreamChannel = outputDevices[0];
	// To get the value in 0.0 to 1.0 range divide the total and current and to invert current * total
	//AudioPlayer->SetPlaybackTime();
	//Song play

	//Initialized = true;
}


void UCGameMusicComp::EmptyParams() {
	NotesSpectrum = NotesSpectrumDefault;
	NotesSpectrumAvg = NotesSpectrumDefault;

	KeyNoteSpectrum = 0.f;

	NotesAmplitude = NotesAmplitudeDefault;

	KeyNoteAmplitude = 0.f;

	KeyPitch = 0.f;

	BeatsStatus = BeatsStatusDefault;
	BeatsAmplitude = BeatsAmplitudeDefault;
	Beats = 0;

	if ( TrackPlaying ) {
		TrackTime = 0.0f;
	}
}

void UCGameMusicComp::DefaultForwardParams() {
	NotesSpectrumForward = NotesSpectrum;
	NotesSpectrumAvgForward = NotesSpectrumAvg;

	KeyNoteSpectrumForward = KeyNoteSpectrum;

	NotesAmplitudeForward = NotesAmplitude;

	KeyNoteAmplitudeForward = KeyNoteAmplitude;

	KeyPitchForward = KeyPitch;
	BeatsStatusForward = BeatsStatus;
	BeatsAmplitudeForward = BeatsAmplitude;
	BeatsForward = Beats;
}

void UCGameMusicComp::PlayerParams() {
	AudioPlayer->GetSpectrum( NotesSpectrum, NotesSpectrumAvg, true );

	float specTotal = 0;
	float specAvgTotal = 0;
	for ( int32 i = 0; i < NotesSpectrumNum; i++ ) {
		specTotal += NotesSpectrum[i];
		specAvgTotal += NotesSpectrumAvg[i];
	}

	KeyNoteSpectrum = specTotal / NotesSpectrumNum;
	KeyNoteSpectrumAvg = specAvgTotal / NotesSpectrumNum;

	// Amplitudes are over time
	AudioPlayer->GetAmplitude( NotesAmplitude );

	KeyNoteAmplitude = NotesAmplitude[KeyNoteAmplitudeIndex];

	AudioPlayer->GetPitchTracking( KeyPitch );

	AudioPlayer->GetBeatTracking( BeatsStatus[0], BeatsStatus[1], BeatsStatus[2], BeatsAmplitude, BeatsPM, NullDummyInt );

	float count = 0.f;
	bool beatCheck = false;
	int32 beatMainIndex = 0;
	for ( int32 i = BeatsAmplitude.Num()-1; i >= 0; i-- ) {
		if ( BeatsStatus[i] && !beatCheck ) {
			beatMainIndex = i;
			beatCheck = true;
		}
		count += BeatsPM[i];
	}
	Beats = FMath::TruncToInt( count / BeatsPM.Num() );

	if ( beatCheck ) { // Call it after process 
		OnBeat.Broadcast( beatMainIndex, BeatsAmplitude[beatMainIndex], KeyPitch );
	}

	if ( TrackPlaying ) {
		TrackTime = AudioPlayer->GetPlaybackTime();
	}
}

void UCGameMusicComp::ForwardPlayerParams() {

	if ( AudioPlayerForward->IsPlaying() ) {
		TArray<int32> beatsAmplitude;

		AudioPlayerForward->GetSpectrum( NotesSpectrumForward, NotesSpectrumAvgForward, false );

		float specTotal = 0;
		float specAvgTotal = 0;
		for ( int32 i = 0; i < NotesSpectrumNum; i++ ) {
			specTotal += NotesSpectrumForward[i];
			specAvgTotal += NotesSpectrumAvgForward[i];
		}

		KeyNoteSpectrumForward = specTotal / NotesSpectrumNum;
		KeyNoteSpectrumAvgForward = specAvgTotal / NotesSpectrumNum;

		// Amplitudes are over time
		AudioPlayerForward->GetAmplitude( NotesAmplitudeForward );

		KeyNoteAmplitudeForward = NotesAmplitudeForward[KeyNoteAmplitudeIndex];

		AudioPlayerForward->GetPitchTracking( KeyPitchForward );

		AudioPlayerForward->GetBeatTracking( BeatsStatusForward[0], BeatsStatusForward[1], BeatsStatusForward[2], BeatsAmplitudeForward, BeatsPMForward, NullDummyInt );

		float count = 0.f;
		bool beatCheck = false;
		int32 beatMainIndex = 0;
		for ( int32 i = BeatsAmplitudeForward.Num()-1; i >= 0; i-- ) {
			if ( BeatsStatusForward[i] && !beatCheck ) {
				beatMainIndex = i;
				beatCheck = true;
			}
			count += BeatsPMForward[i];
		}
		Beats = FMath::TruncToInt( count / BeatsPMForward.Num() );

		if ( beatCheck ) { // Call it after process 
			OnBeatForward.Broadcast( beatMainIndex, BeatsAmplitudeForward[beatMainIndex], KeyPitchForward );
		}

		if ( TrackForwardPlaying ) {
			TrackForwardTime = AudioPlayerForward->GetPlaybackTime();
		}
	}
}

void UCGameMusicComp::UpdateParams() {
	LocalTime = FDateTime::UtcNow();
	OnlineTime = FDateTime::Now();

	if ( InGame ) {
		if ( !AudioPlayer && !AudioPlayerForward ) {
			return;
		}

		if ( Device == EMusicDeviceList::eSingle ) {
			if ( AudioPlayer->IsPlaying() ) {
				ForwardPlayerParams();
				PlayerParams();

			} else {
				EmptyParams();
				DefaultForwardParams();
			}
		}

	} else {
		if ( !AudioPlayer && !AudioPlayerForward && !MediaPlayer && !MediaSoundComponent ) {
			return;
		}

		if ( Device == EMusicDeviceList::eRadioNet ) {
			if ( MediaPlayer->IsPlaying() ) {
				TArray<FMediaSoundComponentSpectralData> arrayList = MediaSoundComponent->GetSpectralData();
				int32 arrayNum = arrayList.Num();
				float specTotal = 0;
				float specAvgTotal = 0;
				bool chk = true;
				int32 cnt = 0;
				for ( int32 i = 0; i < arrayNum; i++ ) {
					specTotal = +arrayList[i].Magnitude;
					NotesSpectrum[i] = arrayList[i].Magnitude;
				}
				KeyNoteSpectrum = specTotal / NotesSpectrumNum;
				KeyNoteAmplitude = KeyNoteSpectrum;

				NotesAmplitude = NotesAmplitudeDefault;

				KeyPitch = 0.f;

				BeatsStatus = BeatsStatusDefault;
				BeatsAmplitude = BeatsAmplitudeDefault;
				Beats = 0;

				if ( TrackPlaying ) {
					TrackTime = 0.f;
				}
				DefaultForwardParams();
			} else {
				EmptyParams();
				DefaultForwardParams();
			}
		} else if (Device == EMusicDeviceList::eListen) {
			if (AudioPlayer->IsPlaying()) {
				PlayerParams();
			} else {
				EmptyParams();
			}
		} else if ( Device == EMusicDeviceList::eSingle ) {
			if ( AudioPlayer->IsPlaying() ) {
				ForwardPlayerParams();
				PlayerParams();

			} else {
				EmptyParams();
				DefaultForwardParams();
			}
		} else {
			// Playlists Ones
			if ( AudioPlayer->IsPlaying() ) {
				PlayerParams();
				ForwardPlayerParams();
				//DefaultForwardParams();
			} else {
				EmptyParams();
				DefaultForwardParams();
			}
		}
	}

	if ( WorldParams != nullptr ) {
		for ( int32 i = 0; i < NotesSpectrumNum; i++ ) {
			FString lStr = "NotesSpectrum" + FString::FromInt( i );
			UKismetMaterialLibrary::SetScalarParameterValue( GetWorld(), WorldParams, FName( *lStr ), NotesSpectrum[i] );
		}
		UKismetMaterialLibrary::SetScalarParameterValue( GetWorld(), WorldParams, FName( "KeyNoteSpectrum" ), KeyNoteSpectrum );
		//UKismetMaterialLibrary::SetScalarParameterValue( GetWorld(), WorldParams, FName( "" ), KeyNoteSpectrumAvg );

		for ( int32 i = 0; i < NotesAmplitudeNum; i++ ) {
			FString lStr = "NotesAmplitude" + FString::FromInt( i );
			UKismetMaterialLibrary::SetScalarParameterValue( GetWorld(), WorldParams, FName( *lStr ), NotesAmplitude[i] );
		}
		UKismetMaterialLibrary::SetScalarParameterValue( GetWorld(), WorldParams, FName( "KeyNoteAmplitude" ), KeyNoteAmplitude );
	}
}


void UCGameMusicComp::Reset_Implementation() {
	if ( GetWorld() ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {

		}
	}
}

void UCGameMusicComp::PlayLogic() {

	if (Device == EMusicDeviceList::eRadioNet) {
		if (TrackEnded && !TrackStarted) { // Finished Playing last
			TrackTime = 0.f;
			Paused = false;
			Stopped = false;
			MediaPlayer->Pause(); //Just in case, need to fix the reload
			MediaPlayer->Play();
			OnTrackStart();
		} else { // Replay
			Paused = false;
			Stopped = false;
			MediaPlayer->Play();
			OnTrackStart();
		}
	} else if (Device == EMusicDeviceList::eListen) {
		if (TrackEnded && !TrackStarted) { // Finished Playing last
			TrackTime = 0.f;
			Paused = false;
			Stopped = false;
			AudioPlayer->StopLoopback(); //Just in case, need to fix the reload
			AudioPlayer->StartLoopback();
			OnTrackStart();
		} else { // Replay
			Paused = false;
			Stopped = false;
			AudioPlayer->StartLoopback();
			TrackPlaying = true;
			if (TrackTime <= 0.f) {
				OnTrackStart();
			}
		}
	} else {
		if ( TrackEnded && !TrackStarted ) { // Finished Playing last
			TrackTime = 0.f;
			Paused = false;
			Stopped = false;
			AudioPlayer->Stop(); //Just in case, need to fix the reload
			SetTime( TrackTime );
			AudioPlayer->Play();
			OnTrackStart();
			OnTrackForwardStart();
		} else { // Replay
			Paused = false;
			Stopped = false;
			AudioPlayer->Play(); 
			TrackPlaying = true;
			if ( TrackTime <= 0.f ) {
				OnTrackStart();
				OnTrackForwardStart();
			}
		}
	}
}


void UCGameMusicComp::Play( bool _AutoManage ) {
	if (Device == EMusicDeviceList::eRadioNet) {
		if (MediaPlayer) {
			if (TrackValid) {
				UE_LOG(LogTemp, Warning, TEXT("Play Radio - Valid Track"));
				if (TrackPlaying) {
					if (_AutoManage) {
						Pause();
					}
					UE_LOG(LogTemp, Warning, TEXT("Play Radio - Track Already Playing"));
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Play Radio - Track Not Playing"));
					if (TrackEnded && !TrackStarted) {
						PlayLogic();
						UE_LOG(LogTemp, Warning, TEXT("Play Radio - Track Ended"));
					}
					else {
						PlayLogic();
						UE_LOG(LogTemp, Warning, TEXT("Play Radio - Track Started"));
					}
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Play Radio - Invalid Track"));
			}
			//OnTrackUpdate.Broadcast();
		}
	} else if (Device == EMusicDeviceList::eListen) {
		if (AudioPlayer) {
			if (TrackValid) {
				UE_LOG(LogTemp, Warning, TEXT("Play - Valid STREAM"));
				if (TrackPlaying) {
					if (_AutoManage) {
						Pause();
					}
					UE_LOG(LogTemp, Warning, TEXT("Play - STREAM Already Playing"));
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Play - STREAM Not Playing"));
					if (TrackEnded && !TrackStarted) {
						PlayLogic();
						UE_LOG(LogTemp, Warning, TEXT("Play - STREAM Ended"));
					}
					else {
						PlayLogic();
						UE_LOG(LogTemp, Warning, TEXT("Play - STREAM Started"));
					}
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Play - Invalid STREAM"));
			}
			OnTrackUpdate.Broadcast();
		}
	} else {
		if (AudioPlayer) {
			if (TrackValid) {
				UE_LOG(LogTemp, Warning, TEXT("Play - Valid Track"));
				if (TrackPlaying) {
					if (_AutoManage) {
						Pause();
					}
					UE_LOG(LogTemp, Warning, TEXT("Play - Track Already Playing"));
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Play - Track Not Playing"));
					if (TrackEnded && !TrackStarted) {
						PlayLogic();
						UE_LOG(LogTemp, Warning, TEXT("Play - Track Ended"));
					}
					else {
						PlayLogic();
						UE_LOG(LogTemp, Warning, TEXT("Play - Track Started"));
					}
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Play - Invalid Track"));
			}
			OnTrackUpdate.Broadcast();
		}
	}
}

// Only in game mode
void UCGameMusicComp::PlayForwardLogic() {

	if ( TrackForwardEnded && !TrackForwardStarted ) { // Finished Playing last
		TrackForwardTime = 0.f;
		AudioPlayerForward->Stop(); //Just in case, need to fix the reload
		SetTime( TrackForwardTime );
		AudioPlayerForward->Play();
		OnTrackForwardStart();
	} else { // Replay
		AudioPlayerForward->Play();
		TrackForwardPlaying = true;
		if ( TrackForwardTime <= 0.f ) {
			OnTrackForwardStart();
		} else {

		}
	}
}

void UCGameMusicComp::PlayForward( bool _AutoManage ) {
	if ( Device != EMusicDeviceList::eRadioNet ) {
		if ( ForwardPlayer ) {
			if ( TrackValid ) {
				UE_LOG( LogTemp, Warning, TEXT( "PlayForward() - Valid Track" ) );
				if ( TrackForwardPlaying ) {
					if ( _AutoManage ) {
						Pause();
					}
					UE_LOG( LogTemp, Warning, TEXT( "PlayForward() - Track Already Playing" ) );
				} else {
					UE_LOG( LogTemp, Warning, TEXT( "PlayForward() - Track Not Playing" ) );
					if ( TrackForwardEnded && !TrackForwardStarted ) {
						PlayForwardLogic();
						UE_LOG( LogTemp, Warning, TEXT( "PlayForward() - Track Ended" ) );
					} else {
						PlayForwardLogic();
						UE_LOG( LogTemp, Warning, TEXT( "PlayForward() - Track Started" ) );
					}
				}
			} else {
				UE_LOG( LogTemp, Warning, TEXT( "PlayForward() - Invalid Track" ) );
			}
			//OnTrackUpdate.Broadcast();
		}
	}
}


void UCGameMusicComp::SetTime( const float _Time ) {
	if ( Device == EMusicDeviceList::eRadioLocal || Device == EMusicDeviceList::eRadioNet || Device == EMusicDeviceList::eListen ) {
		return;
	}
	float error = .99f;
	if ( AudioPlayer ) {
		if ( AudioPlayerForward ) {
			float time = FMath::Clamp( _Time + ForwardPlayerTime, 0.0f, TrackLength - error );

			if ( time >= TrackLength ) {
				TrackTime = 0.0f;
				AudioPlayerForward->Pause();
				AudioPlayerForward->Stop();
				OnTrackForwardEnd();
				// TODO OnMain end
			} else {
				TrackTime = time;
				AudioPlayer->SetPlaybackTime( time );
			}
		}


		float time = FMath::Clamp( _Time, 0.0f, TrackLength - error );

		if ( time >= TrackLength ) {
			TrackTime = 0.0f;
			AudioPlayer->Pause();
			AudioPlayer->Stop();
			OnTrackEnd();
		} else {
			TrackTime = time;
			AudioPlayer->SetPlaybackTime( time );
		}
		OnTrackUpdate.Broadcast();
	}
}

void UCGameMusicComp::FastForward() {
	if ( Device == EMusicDeviceList::eRadioLocal || Device == EMusicDeviceList::eRadioNet || Device == EMusicDeviceList::eListen) {
		return;
	}
	float dTime = 20.0f;
	float time = TrackTime + dTime;
	time = FMath::Clamp( time, 0.f, TrackLength );
	SetTime( time );
}

void UCGameMusicComp::FastBackward() {
	if ( Device == EMusicDeviceList::eRadioLocal || Device == EMusicDeviceList::eRadioNet || Device == EMusicDeviceList::eListen) {
		return;
	}
	float dTime = 20.0f;
	float time = TrackTime - dTime;
	time = FMath::Clamp( time, 0.f, TrackLength );
	SetTime( time );
}

void UCGameMusicComp::Rewind() {
	if ( Device == EMusicDeviceList::eRadioLocal || Device == EMusicDeviceList::eRadioNet || Device == EMusicDeviceList::eListen ) {
		return;
	}
	SetTime( 0.0f );
}

void UCGameMusicComp::SetTrackLoop( const bool _TrackLoop ) {
	TrackLoopPlay = _TrackLoop;
	SetInstanceDevicePlaylist();
}

bool UCGameMusicComp::GetTrackLoop() {
	return TrackLoopPlay;
}

void UCGameMusicComp::Pause() {
	if ( Stopped ) {
		return;
	}
	if (Device == EMusicDeviceList::eRadioNet) {
		if (MediaPlayer) {
			MediaPlayer->Pause();
			TrackPlaying = false;
			Paused = true;
			OnTrackUpdate.Broadcast();
		}
	} else if (Device == EMusicDeviceList::eListen) {
		if (AudioPlayer) {
			AudioPlayer->StopLoopback();
			TrackPlaying = false;
			Paused = true;
			OnTrackUpdate.Broadcast();
		}
	} else {
		if (AudioPlayer) {
			if (AudioPlayerForward && ForwardPlayer) {
				AudioPlayerForward->Pause();
				TrackForwardPlaying = false;
			}
			AudioPlayer->Pause();
			TrackPlaying = false;
			Paused = true;
			OnTrackUpdate.Broadcast();
		}
	}
}

void UCGameMusicComp::Stop() {
	if (Device == EMusicDeviceList::eListen) {
		if (AudioPlayer) {
			AudioPlayer->StopLoopback();
			TrackPlaying = false;
			TrackEnded = true;
			TrackStarted = false;
			Stopped = true;
		}
	} else {
		if (AudioPlayer) {
			if (AudioPlayerForward && ForwardPlayer) {
				AudioPlayerForward->Stop();
				TrackForwardPlaying = false;
				TrackForwardEnded = true;
				TrackForwardStarted = false;
			}
			AudioPlayer->Stop();
			TrackPlaying = false;
			TrackEnded = true;
			TrackStarted = false;
			Stopped = true;
			Rewind();
		}
	}
	if ( MediaPlayer ) {
		MediaPlayer->Pause();
		TrackPlaying = false;
		TrackEnded = true;
		TrackStarted = false;
		Stopped = true;
	}
	OnTrackUpdate.Broadcast();
}

void UCGameMusicComp::SetDevice( const EMusicDeviceList _Device ) {
	if ( _Device != EMusicDeviceList::eSingle || _Device != EMusicDeviceList::eListen ) {
		if ( _Device == Device ) {
			return;
		}
	}

	if ( ThreadLoading ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::SetDevice() Thread In Use" ) );
		return;
	}
	ThreadLoading = true;
	OnThread.Broadcast( true );

	ResetGlobalVariables();

	if ( !ThreadLoadingPlaylists ) {
		Device = _Device;
		ThreadLoadingPlaylists = true;
		ReloadDevicePlaylists( Device );
		ThreadLoadingPlaylists = false;
	}
	ThreadLoading = false;
	OnThread.Broadcast( false );
}

EMusicDeviceList UCGameMusicComp::GetDevice() {
	return Device;
}

bool UCGameMusicComp::SetPlaylistIndex( FThreadLoadedBP _OnLoadComplete, const int32 _Index, const int32 _TrackIndex, const bool _AutoPlay ) {
	if ( ThreadLoading ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::SetPlaylistIndex() Thread In Use" ) );
		return false;
	}

	int32 devidePLNum = GetDevicePlaylistNum( Device );
	if ( devidePLNum <= 0 ) { // Empty
		return false;
	} else {
		TrackAutoPlayOnLoad = _AutoPlay;
		if ( !ThreadLoadingTracks ) {
			ThreadLoadingTracks = true;
			OnThreadLoadingTracks.Broadcast();
			int32 playlistIndex = 0;
			int32 trackIndex = _TrackIndex;
			if ( _Index < devidePLNum ) {
				playlistIndex = _Index;
			} else {
				playlistIndex = 0;
			}
			if ( PlaylistIndex != playlistIndex || LastDevice != Device || Device == EMusicDeviceList::eSingle || Device == EMusicDeviceList::eListen ) {
				UE_LOG( LogTemp, Warning, TEXT( "THREADED" ) );

				AsyncTask( ENamedThreads::AnyThread, [&, _OnLoadComplete, _AutoPlay, playlistIndex, trackIndex]() {
					ReloadPlaylistTracks( this->Device, playlistIndex );

					AsyncTask( ENamedThreads::GameThread, [&, _OnLoadComplete, _AutoPlay, playlistIndex, trackIndex]() {
						this->ThreadLoadingTracks = false;
						if ( !this->ThreadLoadingPlaylists && !this->ThreadLoadingTracks ) {
							this->Stop();
							int32 playlistIndexLocal = playlistIndex;
							int32 trackIndexLocal = trackIndex;
							trackIndexLocal = FMath::Min( this->GetPlaylistTracksNum( this->Device, playlistIndexLocal ) - 1, trackIndexLocal );
							trackIndexLocal = FMath::Max( 0, trackIndexLocal );

							this->TrackIndex = trackIndexLocal;
							this->PlaylistIndex = playlistIndexLocal;
							this->PlaylistTracksNum = this->GetPlaylistTracksNum( this->Device, this->PlaylistIndex );
							FPlaylistStruct info = this->GetPlaylistInfo( this->Device, this->PlaylistIndex );
							this->PlaylistName = info.Name;

							this->SetInstanceDevicePlaylist();

							this->LastDevice = this->Device;
							this->OnThreadLoadedTracks.Broadcast();
							//return true;
						}
						if ( _OnLoadComplete.IsBound() ) {
							_OnLoadComplete.Execute();
						}
					} );
				} );
				return true;
			} else {
				UE_LOG( LogTemp, Warning, TEXT( "unTHREADED" ) );
				ThreadLoadingTracks = false;

				TrackIndex = trackIndex;
				PlaylistIndex = playlistIndex;

				PlaylistTracksNum = GetPlaylistTracksNum( Device, PlaylistIndex );
				FPlaylistStruct info = GetPlaylistInfo( Device, PlaylistIndex );
				PlaylistName = info.Name;

				SetInstanceDevicePlaylist();
				LastDevice = Device;
				OnThreadLoadedTracks.Broadcast();

				if ( _OnLoadComplete.IsBound() ) {
					_OnLoadComplete.Execute();
				}
				return true;
			}
		} else {
			return false;
		}
	}
}

int32 UCGameMusicComp::GetPlaylist( FString& _Name, int32& _TracksNum, int32& _TrackIndex ) {
	_Name = PlaylistName;
	_TracksNum = PlaylistTracksNum;
	_TrackIndex = TrackIndex;
	return PlaylistIndex;
}


bool UCGameMusicComp::SetTrackIndex( const int32 _Index, const bool _AutoPlay ) {
	if ( ThreadLoading || ThreadLoadingTracks ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::SetTrackIndex() Thread In Use" ) );
		return false;
	}
	TrackAutoPlayOnLoad = _AutoPlay;

	if (Device == EMusicDeviceList::eRadioNet) {
		//TODO
		UE_LOG(LogTemp, Warning, TEXT("Set Track to Net Radio"));


		FRadioNetStationStruct radio = GetRadioNetInfo(PlaylistIndex);
		if (radio.URL != "") {
			TrackIndex = 0;
			SetLoadNetRadioData(radio);
		}
		else {
			ResetGlobalVariables();
		}
	} else if (Device == EMusicDeviceList::eListen) {
		FTrackStruct track = GetTrackInfo(Device, 0, 0);
		if (track.Format == ETrackFormats::eSERROR || track.URL == "") {
			UE_LOG(LogTemp, Warning, TEXT("SetTrackIndex - Invalid STREAM"));
			UE_LOG(LogTemp, Warning, TEXT("Track STREAM: %s"), *track.URL);
			// INVALID
			if (Track.Format == ETrackFormats::eSERROR || Track.URL == "") {
				ResetGlobalVariables();
			}
			else {
				SetLoadTrackData(Track);
			}
		}
		else {
			TrackIndex = 0;
			SetLoadTrackData(track);
		}
	} else {
		FTrackStruct track = GetTrackInfo( Device, PlaylistIndex, _Index );
		if ( track.Format == ETrackFormats::eSERROR || track.URL == "" ) {
			UE_LOG( LogTemp, Warning, TEXT( "SetTrackIndex - Invalid Track" ) );
			UE_LOG( LogTemp, Warning, TEXT( "Track URL: %s" ), *track.URL );
			// INVALID
			if ( Track.Format == ETrackFormats::eSERROR || Track.URL == "" ) {
				ResetGlobalVariables();
			} else {
				SetLoadTrackData( Track );
			}
		} else {
			TrackIndex = _Index;
			SetLoadTrackData( track );
		}
	}
	return TrackValid;
}

bool UCGameMusicComp::SetTrackRandomIndex( const bool _AutoPlay ) {
	int32 numSongs = GetPlaylistTracksNum( Device, PlaylistIndex );
	if ( numSongs <= 0 ) {
		// INVALID
		return false;
	}
	numSongs = FMath::Max( 0, numSongs - 1 );
	int32 rN = FMath::RandRange( 0, numSongs );

	return SetTrackIndex( rN, _AutoPlay );

}

bool UCGameMusicComp::CheckTrackStatus() {
	LastTrackPlaying = TrackPlaying;
	TrackPlaying = false;

	if ( LastTrackPlaying ) {
		return true;
	} else {
		return false;
	}
}


void UCGameMusicComp::Next() {
	if ( Device != EMusicDeviceList::eCassette && Device != EMusicDeviceList::ePurpleRay ) {
		return;
	}
	if ( TrackValid ) {
		int32 deviceTracksNum = GetPlaylistTracksNum( Device, PlaylistIndex );
		int32 index = TrackIndex + 1;
		if ( index >= deviceTracksNum ) { index = 0; }
		bool play = TrackPlaying;
		CheckTrackStatus();
		//TrackPlaying = CheckTrackStatus();
		SetTrackIndex( index, play ); //CheckTrackStatus()
	}
}

void UCGameMusicComp::Prev() {
	if ( Device != EMusicDeviceList::eCassette && Device != EMusicDeviceList::ePurpleRay ) {
		return;
	}
	if ( TrackValid ) {
		int32 deviceTracksNum = GetPlaylistTracksNum( Device, PlaylistIndex );
		int32 index = TrackIndex - 1;
		int32 lastIndex = FMath::Max( 0, deviceTracksNum - 1);
		if ( index < 0 ) { index = lastIndex; }
		bool play = TrackPlaying;
		CheckTrackStatus();
		//TrackPlaying = CheckTrackStatus();
		SetTrackIndex( index, TrackPlaying ); //CheckTrackStatus()
	}
}

void UCGameMusicComp::NextPlaylist() {
	if ( Device == EMusicDeviceList::eSingle ) {
		SetDevice( EMusicDeviceList::eCassette );
	}
	int32 devicePlaylistNum = GetDevicePlaylistNum( Device );
	int32 index = PlaylistIndex + 1;
	if ( index >= devicePlaylistNum ) { index = 0; }

	SetPlaylistIndex( FThreadLoadedBP{}, index );
}

void UCGameMusicComp::PrevPlaylist() {
	if ( Device == EMusicDeviceList::eSingle ) {
		SetDevice( EMusicDeviceList::eCassette );
	}
	int32 devicePlaylistNum = GetDevicePlaylistNum( Device );
	int32 index = PlaylistIndex - 1;
	int32 lastIndex = FMath::Max( 0, devicePlaylistNum - 1 );
	if ( index < 0 ) { index = lastIndex; }

	SetPlaylistIndex( FThreadLoadedBP{}, index );
}

void UCGameMusicComp::SetAutoPlay( const bool _AutoPlay ) {
	PlaylistAutoPlay = _AutoPlay;
	SetInstanceDevicePlaylist();
}

bool UCGameMusicComp::GetAutoPlay() {
	return PlaylistAutoPlay;
}

void UCGameMusicComp::SetRandomPlay( const bool _RandomPlay ) {
	PlaylistRandomOrder = _RandomPlay;
	SetInstanceDevicePlaylist();
}

bool UCGameMusicComp::GetRandomPlay() {
	return PlaylistRandomOrder;
}

bool GetCoverImage( UTexture2D* _Texture, const FString _URL ) {
	if ( !FPaths::FileExists( _URL ) ) { return false; }
	bool valid = false;
	int width, height;
	UTexture2D* texture = UHEVLibraryMedia::LoadTexture2D_FromFile( _URL, EHEVListImageFormats::PNG, valid, width, height );
	if ( !valid ) {
		texture = UHEVLibraryMedia::LoadTexture2D_FromFile( _URL, EHEVListImageFormats::JPG, valid, width, height );
	}
	if ( valid ) {
		_Texture = texture;
	}
	return valid;
}

TArray<FTrackStruct> UCGameMusicComp::GetTracks( const TArray<FTrackStruct>& _TrackFiles, const int32 _Limit, const bool _Soft ) {
	UE_LOG( LogTemp, Warning, TEXT( "GetTracks(): GET TRACKS INFO" ) );

	TArray<FTrackStruct> songList;
	const int32 maxIndex = FMath::Min( _TrackFiles.Num(), _Limit );

	if ( _Soft ) {
		UE_LOG( LogTemp, Warning, TEXT( "GetTracks(): Soft" ) );
		for ( int32 i = 0; i < maxIndex; i++ ) {
			FTrackStruct curSong = _TrackFiles[i];
			FString ext, extra;
			UE_LOG( LogTemp, Warning, TEXT( "GetTracks(): Track Info: %s" ), *curSong.URL );
			if ( AudioAnalyzerDecoderFast::GetMetadata( curSong.URL, curSong.Name, ext, extra, curSong.Title, curSong.Artist, curSong.Album, curSong.Year, curSong.Genre, curSong.Length ) ) {
				if ( curSong.Title == "" || curSong.Title == " " ) {
					curSong.Title = curSong.Name;
				}
				//".ogg", ".wav", ".mp3", ".flac"
				if ( ext == ".ogg" ) {
					curSong.Format = ETrackFormats::eOGG;
				} else if ( ext == ".wav" ) {
					curSong.Format = ETrackFormats::eWAV;
				} else if ( ext == ".mp3" ) {
					curSong.Format = ETrackFormats::eMP3;
				} else if ( ext == ".flac" ) {
					curSong.Format = ETrackFormats::eFLAC;
				} else {
					curSong.Format = ETrackFormats::eSERROR;
				}
				//UE_LOG( LogTemp, Warning, TEXT( "Song Size" ) );
				if ( curSong.Format != ETrackFormats::eSERROR &&  curSong.Length > 14.f ) { // Safe lock here
					songList.Add( curSong );
					UE_LOG( LogTemp, Warning, TEXT( "TRACK PASS" ) );
				}
			}
		}
	} else {
		UE_LOG( LogTemp, Warning, TEXT( "GetTracks(): Pure" ) );
		// Audio Decoder
		AudioAnalyzerPlayerDecoder* audioDec = new AudioAnalyzerPlayerDecoder();
		for ( int32 i = 0; i < maxIndex; i++ ) {
			FTrackStruct curSong = _TrackFiles[i];

			if ( audioDec->loadAudioInfo( curSong.URL ) ) {
				FString name, ext, extra, title, artist, album, year, genre;
				audioDec->getMetadata( name, ext, extra, title, artist, album, year, genre );
				curSong.Name = name;
				curSong.Title = title;
				curSong.Artist = artist;
				curSong.Album = album;
				curSong.Year = year;
				curSong.Genre = genre;
				curSong.ID = curSong.Name + "_" + curSong.Album;
				curSong.URL = curSong.URL;

				if ( curSong.Title == "" || curSong.Title == " " ) {
					curSong.Title = curSong.Name;
				}

				//".ogg", ".wav", ".mp3", ".flac"
				ext = FPaths::GetExtension( curSong.URL, true );
				if ( ext == ".ogg" ) {
					curSong.Format = ETrackFormats::eOGG;
				} else if ( ext == ".wav" ) {
					curSong.Format = ETrackFormats::eWAV;
				} else if ( ext == ".mp3" ) {
					curSong.Format = ETrackFormats::eMP3;
				} else if ( ext == ".flac" ) {
					curSong.Format = ETrackFormats::eFLAC;
				} else {
					curSong.Format = ETrackFormats::eSERROR;
				}
				//UE_LOG( LogTemp, Warning, TEXT( "Song Size" ) );
				if ( curSong.Format != ETrackFormats::eSERROR &&  curSong.Length > 14.f ) { // Safe lock here
					songList.Add( curSong );
					UE_LOG( LogTemp, Warning, TEXT( "TRACK PASS" ) );
				}
			} else {

			}

		}
	}
	return songList;
}

// Just for game use
bool UCGameMusicComp::GetTrackMeta( const FTrackStruct _TrackFile, FTrackStruct& _Track ) {
	UE_LOG( LogTemp, Warning, TEXT( "GetTrack(): GET TRACK INFO" ) );
	FTrackStruct curSong = _TrackFile;
	FString ext, extra;
	UE_LOG( LogTemp, Warning, TEXT( "GetTrack(): Track Info: %s" ), *curSong.URL );
	if ( AudioAnalyzerDecoderFast::GetMetadata( curSong.URL, curSong.Name, ext, extra, curSong.Title, curSong.Artist, curSong.Album, curSong.Year, curSong.Genre, curSong.Length ) ) {
		if ( curSong.Title == "" || curSong.Title == " " ) {
			curSong.Title = curSong.Name;
		}
		//".ogg", ".wav", ".mp3", ".flac"
		if ( ext == ".ogg" ) {
			curSong.Format = ETrackFormats::eOGG;
		} else if ( ext == ".wav" ) {
			curSong.Format = ETrackFormats::eWAV;
		} else if ( ext == ".mp3" ) {
			curSong.Format = ETrackFormats::eMP3;
		} else if ( ext == ".flac" ) {
			curSong.Format = ETrackFormats::eFLAC;
		} else {
			curSong.Format = ETrackFormats::eSERROR;
		}
		if ( curSong.Format != ETrackFormats::eSERROR &&  curSong.Length > 14.f ) { // Safe lock here
			UE_LOG( LogTemp, Warning, TEXT( "TRACK Valid" ) );
			_Track = curSong;
			return true;
		}
	}
	UE_LOG( LogTemp, Warning, TEXT( "TRACK Invalid" ) );
	return false;

}


int32 UCGameMusicComp::GetDevicePlaylistNum( const EMusicDeviceList _Device ) {
	int32 num = -1;
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			switch( _Device ) {
				case EMusicDeviceList::eSingle:
					num = 1;
					break;
				case EMusicDeviceList::eListen:
					num = 1;
					break;
				case EMusicDeviceList::eCassette:
					num = lGI->MP_Cassettes.Num();
					break;
				case EMusicDeviceList::ePurpleRay:
					num = lGI->MP_PurpleRays.Num();
					break;
				case EMusicDeviceList::eRadioLocal:
					num = lGI->MP_RadioLocalStations.Num();
					break;
				case EMusicDeviceList::eRadioNet:
					num = lGI->MP_RadioNetStations.Num();
					break;
				default:
					break;
			}
			//num = num - 1;
		}
	}
	return num; //If -1 then there is nothing to play
}

int32 UCGameMusicComp::GetPlaylistTracksNum( const EMusicDeviceList _Device, const int32 _PlaylistIndex ) {
	int32 sDeviceIndex = GetDevicePlaylistNum( _Device );
	if ( sDeviceIndex <= 0 ) { //Defice its empty
		return -1;
	} else if ( _PlaylistIndex > sDeviceIndex-1 ) { // Out of bounds
		return -1;
	} else {

	}

	int32 num = -1;
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			switch ( _Device ) {
				case EMusicDeviceList::eSingle:
					num = 1;
					break;
				case EMusicDeviceList::eListen:
					num = 1;
					break;
				case EMusicDeviceList::eCassette:
					num = lGI->MP_Cassettes[_PlaylistIndex].Tracks.Num();
					break;
				case EMusicDeviceList::ePurpleRay:
					num = lGI->MP_PurpleRays[_PlaylistIndex].Tracks.Num();
					break;
				case EMusicDeviceList::eRadioLocal:
					num = lGI->MP_RadioLocalStations[_PlaylistIndex].Playlist.Tracks.Num();
					break;
				case EMusicDeviceList::eRadioNet:
					num = 1;
					break;
				default:
					break;
			}
			//num = num - 1;
		}
	}
	return num; //If -1 then there is nothing to play
}

FPlaylistStruct UCGameMusicComp::GetPlaylistInfo( const EMusicDeviceList _Device, const int32 _PlaylistIndex ) {
	FPlaylistStruct playlist = {};
	int32 sDeviceIndex = GetDevicePlaylistNum( _Device );
	if ( sDeviceIndex <= 0 ) { //Defice its empty
		return playlist;
	} else if ( _PlaylistIndex > sDeviceIndex - 1 ) { // Out of bounds
		return playlist;
	} else {

	}

	FTrackStruct streamData;
	streamData.Name = "LOCALHOST";
	streamData.URL = "LOCALHOST";

	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			switch ( _Device ) {
				case EMusicDeviceList::eSingle:
					playlist.Name = lGI->MP_SingleTrack.URL;
					playlist.TracksLimit = 1;
					playlist.Tracks = { lGI->MP_SingleTrack };
					playlist.TrackFiles = { lGI->MP_SingleTrack.URL };
					playlist.DeviceType = EMusicDeviceList::eSingle;
					//return playlist;
					break;
				case EMusicDeviceList::eListen:
					playlist.Name = "STREAM";
					playlist.TracksLimit = 1;
					playlist.Tracks = { streamData };
					playlist.TrackFiles = { streamData.URL };
					playlist.DeviceType = EMusicDeviceList::eListen;
					//return playlist;
					break;
				case EMusicDeviceList::eCassette:
					playlist = lGI->MP_Cassettes[_PlaylistIndex];
					break;
				case EMusicDeviceList::ePurpleRay:
					playlist = lGI->MP_PurpleRays[_PlaylistIndex];
					break;
				case EMusicDeviceList::eRadioLocal:
					playlist = lGI->MP_RadioLocalStations[_PlaylistIndex].Playlist;
					break;
				case EMusicDeviceList::eRadioNet:
					playlist.Name = lGI->MP_RadioNetStations[_PlaylistIndex].Name;
					playlist.TracksLimit = 1;
					playlist.TrackFiles = { lGI->MP_RadioNetStations[_PlaylistIndex].URL };
					playlist.DeviceType = EMusicDeviceList::eRadioNet;
					break;
				default:
					break;
			}
			//num = num - 1;
		}
	}
	return playlist; //If -1 then there is nothing to play
}

FRadioNetStationStruct UCGameMusicComp::GetRadioNetInfo( const int32 _PlaylistIndex ) {
	FRadioNetStationStruct info = FRadioNetStationStruct();
	int32 num = GetDevicePlaylistNum( EMusicDeviceList::eRadioNet );
	if ( num <= 0 ) {
		return info;
	}
	if ( _PlaylistIndex > num-1 ) {
		return info;
	}
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			info = lGI->MP_RadioNetStations[_PlaylistIndex];
		}
	}
	return info;
}

FRadioLocalStationStruct UCGameMusicComp::GetRadioLocalInfo( const int32 _PlaylistIndex ) {
	FRadioLocalStationStruct info = FRadioLocalStationStruct();
	int32 num = GetDevicePlaylistNum( EMusicDeviceList::eRadioLocal );
	if ( num <= 0 ) {
		return info;
	}
	if ( _PlaylistIndex > num - 1 ) {
		return info;
	}
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			info = lGI->MP_RadioLocalStations[_PlaylistIndex];
		}
	}
	return info;
}

FTrackStruct UCGameMusicComp::GetTrackSetInfo( FString& _Name, FString& _Title, FString& _Artist, FString& _Album, float& _Length ) {
	_Name = Track.Name;
	_Title = Track.Title;
	_Artist = Track.Artist;
	_Album = Track.Album;
	_Length = Track.Length;
	return Track;
}

float UCGameMusicComp::GetTrackTime( float& _Total, float& _Percentage ) {
	_Total = TrackLength;
	_Percentage = TrackTime / TrackLength;
	return TrackTime;
}


FTrackStruct UCGameMusicComp::GetTrackInfo( const EMusicDeviceList _Device, const int32 _PlaylistIndex, const int32 _TrackIndex ) {
	FTrackStruct info = FTrackStruct();
	int32 playlistLastNum = GetPlaylistTracksNum( _Device, _PlaylistIndex );
	if ( playlistLastNum <= 0 ) { //Defice its empty
		return info;
	} else if ( _TrackIndex > playlistLastNum ) { // Out of bounds
		return info;
	} else {

	}

	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			switch ( _Device ) {
				case EMusicDeviceList::eSingle:
					info = lGI->MP_SingleTrack;
					break;
				case EMusicDeviceList::eListen:
					info.Format = ETrackFormats::eSTREAM;
					info.URL = "LOCALHOST";
					info.Name = "Computer Stream";
					info.Title = "STREAMING";
					break;
				case EMusicDeviceList::eCassette:
					info = lGI->MP_Cassettes[_PlaylistIndex].Tracks[_TrackIndex];
					break;
				case EMusicDeviceList::ePurpleRay:
					info = lGI->MP_PurpleRays[_PlaylistIndex].Tracks[_TrackIndex];
					break;
				case EMusicDeviceList::eRadioLocal:
					info = lGI->MP_RadioLocalStations[_PlaylistIndex].Playlist.Tracks[_TrackIndex];
					break;
				case EMusicDeviceList::eRadioNet:
					info.Format = ETrackFormats::eSTREAM;
					info.URL = lGI->MP_RadioNetStations[_PlaylistIndex].URL;
					info.Name = lGI->MP_RadioNetStations[_PlaylistIndex].Name;
					info.Title = lGI->MP_RadioNetStations[_PlaylistIndex].Name;
					break;
				default:
					break;
			}
			//num = num - 1;
		}
	}
	return info;
}


void UCGameMusicComp::ResetGlobalVariables() {
	TrackStarted = false;
	TrackPlaying = false;
	TrackForwardStarted = false;
	TrackForwardPlaying = false;
	TrackEnded = false;
	TrackForwardEnded = false;
	TrackValid = false;
	TrackIsReady = false;
}

void UCGameMusicComp::SetLoadTrackData( const FTrackStruct _Track ) {
	if ( ThreadLoadingTrack ) {
		UE_LOG( LogTemp, Warning, TEXT( "SetLoadTrackData(): Thread in Use" ) );
		return;
	}
	ThreadLoadingTrack = true;

	ResetGlobalVariables();


	Track = _Track;
	TrackName = _Track.Name;
	TrackTitle = _Track.Title;
	TrackArtist = _Track.Artist;
	TrackURL = _Track.URL;
	if ( AudioPlayer ) {
		AudioPlayer->UnloadCapturerAudio();
		AudioPlayer->UnloadLoopbackAudio();
		AudioPlayer->UnloadPlayerAudio();

		if ( !AudioPlayer->OnPlaybackFinished.IsBound() ) {
			AudioPlayer->OnPlaybackFinished.AddDynamic( this, &UCGameMusicComp::OnTrackFinished );
		}

		AsyncTask( ENamedThreads::AnyThread, [&]() {
			TrackTime = 0.f;

			TArray<FString> outputDevices;
			AudioPlayer->GetOutputAudioDevices( outputDevices );
			AudioPlayer->SetDefaultDevicePlayerAudio( outputDevices[0] );
			AudioPlayer->SetDefaultDeviceLoopbackAudio( outputDevices[0] );
			AudioStreamChannel = outputDevices[0];

			bool valid = false;

			if ( Device == EMusicDeviceList::eRadioLocal || Device == EMusicDeviceList::eRadioNet || Device == EMusicDeviceList::eListen ) {
				UE_LOG(LogTemp, Warning, TEXT("SetLoadTrackData(): INITIALIZE LOOPBACK"));
				valid = AudioPlayer->InitLoopbackAudio();
			} else {
				UE_LOG(LogTemp, Warning, TEXT("SetLoadTrackData(): INITIALIZE PLAYER"));
				valid = AudioPlayer->InitPlayerAudio(TrackURL); //File name here
			}

			if ( valid ) {

				//TODO Selected Hz
				AudioPlayer->InitSpectrumConfig( ESpectrumType::ST_Log, EChannelSelectionMode::All_in_one, 0, NotesSpectrumNum, 0.02f, 30, false, 1 ); //ESpectrumType::ST_Linear
				//AudioPlayer->InitSpectrumConfigWLimits( ESpectrumType::ST_Linear, EChannelSelectionMode::All_in_one, bands, 0.02f, 30, false, 1 );
				AudioPlayer->InitAmplitudeConfig( EChannelSelectionMode::All_in_one, 0, NotesAmplitudeNum, 0.02f );
				AudioPlayer->InitPitchTrackingConfig( EChannelSelectionMode::All_in_one, 0 );
				AudioPlayer->InitBeatTrackingConfig( EChannelSelectionMode::All_in_one, 0 );

				if (Device != EMusicDeviceList::eListen) {
					if (ForwardPlayer && AudioPlayerForward) {
						AudioPlayerForward->UnloadCapturerAudio();
						//AudioPlayerForward->UnloadLoopbackAudio();
						AudioPlayerForward->UnloadPlayerAudio();

						if (!AudioPlayerForward->OnPlaybackFinished.IsBound()) {
							AudioPlayerForward->OnPlaybackFinished.AddDynamic(this, &UCGameMusicComp::OnTrackForwardFinished);
						}

						//AudioPlayerForward->SetDefaultDevicePlayerAudio( outputDevices[0] );

						AudioPlayerForward->InitPlayerAudio(TrackURL);
						//TODO Selected Hz
						AudioPlayerForward->InitSpectrumConfig(ESpectrumType::ST_Log, EChannelSelectionMode::All_in_one, 0, NotesSpectrumNum, 0.02f, 30, false, 1);
						AudioPlayerForward->InitAmplitudeConfig(EChannelSelectionMode::All_in_one, 0, NotesAmplitudeNum, 0.02f);
						AudioPlayerForward->InitPitchTrackingConfig(EChannelSelectionMode::All_in_one, 0);
						AudioPlayerForward->InitBeatTrackingConfig(EChannelSelectionMode::All_in_one, 0);

						AudioPlayerForward->SetPlaybackVolume(0.0f);
					}
				}

				//AudioPlayer->InitSpectrumConfig( ESpectrumType::ST_Linear, EChannelSelectionMode::All_in_one, 0, NotesNum, 30, 0.02f, false, 1 );
				float songTimeTotal = AudioPlayer->GetTotalDuration();
				TrackLength = songTimeTotal;

			}

			AsyncTask( ENamedThreads::GameThread, [&, valid]() {
				if ( valid ) {
					OnTrackLoaded();
				} else {
					OnTrackLoadFailed();
				}
			} );
		} );
	}
}


void UCGameMusicComp::OnTrackLoaded() {
	UE_LOG( LogTemp, Warning, TEXT( "OnTrackLoaded()" ) );
	ThreadLoadingTrack = false;
	TrackValid = true;
	TrackIsReady = true;
	OnTrackUpdate.Broadcast();

	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI != nullptr ) {
		SetVolumeMusic( lGI->GameSettings->AudioVolumeMusic, false );
		SetVolumeAnnouncer( lGI->GameSettings->AudioVolumeAnnouncer, false );
	}

	if ( !InGame ) {
		if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
			//UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
			if ( lGI != nullptr ) {
				if ( lGI->LocalPlayerSocial != nullptr ) {
					lGI->LocalPlayerSocial->SetDiscordMusicInfo( GetDeviceName(), PlaylistName, Track.Title, Track.Artist );
				}
			}
		}
		OnTrackSet.Broadcast( true );

		if ( TrackAutoPlayOnLoad ) {
			Play( false );
		}
	} else {
		OnTrackSet.Broadcast( true );
	}
}

void UCGameMusicComp::OnTrackLoadFailed() {
	UE_LOG( LogTemp, Warning, TEXT( "OnTrackLoadFailed()" ) );
	ThreadLoadingTrack = false;
	//ThreadLoadingTrack = false;
	TrackValid = false;
	TrackIsReady = false;
	OnTrackUpdate.Broadcast();

	if ( !InGame ) {
		OnTrackSet.Broadcast( false );
	} else {
		OnTrackSet.Broadcast( false );
	}
	//OnTrackSet.Broadcast(); // ?
}

void UCGameMusicComp::OnTrackStart() {
	if ( !TrackStarted ) {
		TrackStarted = true;
	}
	TrackPlaying = true;

	//UE_LOG( LogTemp, Warning, TEXT( "SONG STARTED" ) );
	OnTrackStarted.Broadcast();
	OnTrackUpdate.Broadcast();

}

void UCGameMusicComp::OnTrackForwardStart() {
	if ( !TrackForwardStarted ) {
		TrackForwardStarted = true;
		TrackForwardEnded = false;
		TrackForwardPlaying = true;
		if ( !TrackValid ) {
			TrackValid = true;
		}
	}
	//UE_LOG( LogTemp, Warning, TEXT( "SONG STARTED" ) );
	OnTrackForwardStarted.Broadcast();
	//OnTrackUpdate.Broadcast();

}

void UCGameMusicComp::OnTrackFinished() {
	OnTrackEnd();
}

void UCGameMusicComp::OnTrackForwardFinished() {
	OnTrackEnd();
}

void UCGameMusicComp::OnTrackEnd() {
	TrackEnded = true;
	TrackPlaying = false;
	TrackStarted = false;
	Pause();
	Stop();

	//UE_LOG( LogTemp, Warning, TEXT( "SONG ENDED" ) );
	OnTrackEnded.Broadcast();
	if ( Device == EMusicDeviceList::eCassette ) {

	} else if ( Device == EMusicDeviceList::ePurpleRay ) {

	} else if ( Device == EMusicDeviceList::eRadioLocal ) {
		SetTrackRandomIndex( true );
	} else {

	}
	OnTrackUpdate.Broadcast();
}

void UCGameMusicComp::OnTrackForwardEnd() {
	TrackForwardEnded = true;
	TrackForwardPlaying = false;
	TrackForwardStarted = false;
	Pause();
	Stop();

	//UE_LOG( LogTemp, Warning, TEXT( "SONG ENDED" ) );
	OnTrackForwardEnded.Broadcast();
}

void UCGameMusicComp::SetLoadNetRadioData( const FRadioNetStationStruct _Radio ) {
	if ( ThreadLoadingTrack ) {
		UE_LOG( LogTemp, Warning, TEXT( "SetLoadNetRadioData(): Thread in Use" ) );
		return;
	}
	//ThreadLoadingTrack = true; Too bad but cannot be handled at all

	ResetGlobalVariables();


	//Track = _Track;
	//Radio = _Radio;
	RadioNet = _Radio;
	TrackName = _Radio.Name;
	TrackTitle = _Radio.Name;
	//TrackArtist = _Track.Artist;
	TrackURL = _Radio.URL;

	RadioName = _Radio.Name;
	RadioURL = _Radio.URL;

	if ( MediaPlayer ) {
		MediaPlayer->Close();

		TrackTime = 0.f;
		TrackIndex = 0;
		//MediaPlayer->Reopen();
		if ( !MediaPlayer->OnMediaOpened.IsBound() ) {
			MediaPlayer->OnMediaOpened.AddDynamic( this, &UCGameMusicComp::OnRadioLoaded );
		}
		if ( !MediaPlayer->OnMediaOpenFailed.IsBound() ) {
			MediaPlayer->OnMediaOpenFailed.AddDynamic( this, &UCGameMusicComp::OnRadioLoadFailed );
		}
		if ( !MediaPlayer->OnEndReached.IsBound() ) {
			MediaPlayer->OnEndReached.AddDynamic( this, &UCGameMusicComp::OnRadioFinished );
		}
		if ( !MediaPlayer->CanPlayUrl( _Radio.URL ) ) {
			UE_LOG( LogTemp, Error, TEXT( "CANNOT PLAY SOURCE" ) );
		}
		if ( !MediaPlayer->OpenUrl( _Radio.URL ) ) {
			OnRadioLoadFailed( _Radio.URL );
		}
		//BindSP for Widgets
		TrackLength = 0.f;
	}
	OnTrackUpdate.Broadcast();
}


//OnMediaOpened
void UCGameMusicComp::OnRadioLoaded( FString OpenedUrl ) {
	UE_LOG( LogTemp, Warning, TEXT( "OnRadioLoaded()" ) );
	//ThreadLoadingTrack = false;
	TrackValid = true;
	TrackIsReady = true;
	OnTrackUpdate.Broadcast();

	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI != nullptr ) {
		SetVolumeMusic( lGI->GameSettings->AudioVolumeMusic, false );
		SetVolumeAnnouncer( lGI->GameSettings->AudioVolumeAnnouncer, false );
	}

	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		//UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			if ( lGI->LocalPlayerSocial != nullptr ) {
				lGI->LocalPlayerSocial->SetDiscordMusicInfo( GetDeviceName(), PlaylistName, Track.Title, Track.Artist );
			}
		}
	}
	OnTrackSet.Broadcast( true );

	if ( TrackAutoPlayOnLoad ) {
		Play( false );
	}
}

//OnMediaOpenFailed
void UCGameMusicComp::OnRadioLoadFailed( FString FailedUrl ) {
	UE_LOG( LogTemp, Warning, TEXT( "OnRadioLoadFailed()" ) );
	//ThreadLoadingTrack = false;
	TrackValid = false;
	TrackIsReady = false;
	OnTrackUpdate.Broadcast();
	//OnTrackSet.Broadcast( false ); // ?
}

void UCGameMusicComp::OnRadioFinished() {
	OnTrackEnd();
}

bool UCGameMusicComp::GetPlayButtonStatus() {
	if ( TrackValid && TrackIsReady ) {
		if ( TrackPlaying ) {
			return true;
		} else {
			return false;
		}
	} else {
		return true;
	}
}

USoundCue* UCGameMusicComp::GetSoundEffect( EPlayerState _State ) {
	USoundCue* sfx = nullptr;
	switch ( _State ) {
		case EPlayerState::PLAY:
			sfx = ( Device == EMusicDeviceList::eCassette ) ? AudioStartRadio : AudioStartRadio;
			break;
		case EPlayerState::PAUSE:
			break;
		case EPlayerState::STOP:
			break;
		case EPlayerState::FASTFORWARD:
			break;
		case EPlayerState::FASTBACKWARD:
			break;
		case EPlayerState::NEXT:
			break;
		case EPlayerState::PREV:
			break;
		case EPlayerState::NONE:
			break;
		default:
			break;
	}
	return sfx;
}

void UCGameMusicComp::PlaySoundEffect( EPlayerState _State ) {

	USoundCue* sfx = GetSoundEffect( _State );
	//TODO

}

void UCGameMusicComp::PlayRadioAd() {

}


float UCGameMusicComp::GetNoteSpectrum( const int32 _NoteIndex, const bool _ForwardPlayer, const bool _Validation ) {
	int32 note = _NoteIndex;
	if ( _Validation ) { note = FMath::Clamp( note, 0, NotesSpectrumLast ); }
	if ( !_ForwardPlayer ) {
		return NotesSpectrum[note];
	} else {
		return NotesSpectrumForward[note];
	}

}

float UCGameMusicComp::GetNoteSpectrumAvg( const int32 _NoteIndex, const bool _ForwardPlayer, const bool _Validation ) {
	int32 note = _NoteIndex;
	if ( _Validation ) { note = FMath::Clamp( note, 0, NotesSpectrumLast ); }
	if ( !_ForwardPlayer ) {
		return NotesSpectrumAvg[note];
	} else {
		return NotesSpectrumAvgForward[note];
	}
}

float UCGameMusicComp::GetNoteAmplitude( const int32 _NoteIndex, const bool _ForwardPlayer, const bool _Validation ) {
	int32 note = _NoteIndex;
	if ( _Validation ) { note = FMath::Clamp( note, 0, NotesAmplitudeLast ); }
	if ( !_ForwardPlayer ) {
		return NotesAmplitude[note];
	} else {
		return NotesAmplitudeForward[note];
	}
}

TArray<float> UCGameMusicComp::GetNotesSpectrum( const TArray<int32> _NotesIndexs, const bool _ForwardPlayer, const bool _Validation ) {
	TArray<int32> notes = _NotesIndexs;
	TArray<float> notesFinal;
	const int32 lastIndex = NotesSpectrumLast;
	if ( _Validation ) {
		for ( int32 note : notes ) {
			note = FMath::Min( note, lastIndex );
			if ( !_ForwardPlayer ) {
				notesFinal.Add( NotesSpectrum[note] );
			} else {
				notesFinal.Add( NotesSpectrumForward[note] );
			}
		}
	} else {
		for ( int32 note : notes ) {
			if ( !_ForwardPlayer ) {
				notesFinal.Add( NotesSpectrum[note] );
			} else {
				notesFinal.Add( NotesSpectrumForward[note] );
			}
		}
	}
	return notesFinal;
}

TArray<float> UCGameMusicComp::GetNotesSpectrumAvg( const TArray<int32> _NotesIndexs, const bool _ForwardPlayer, const bool _Validation ) {
	TArray<int32> notes = _NotesIndexs;
	TArray<float> notesFinal;
	const int32 lastIndex = NotesSpectrumLast;
	if ( _Validation ) {
		for ( int32 note : notes ) {
			note = FMath::Min( note, lastIndex );
			if ( !_ForwardPlayer ) {
				notesFinal.Add( NotesSpectrumAvg[note] );
			} else {
				notesFinal.Add( NotesSpectrumAvgForward[note] );
			}
		}
	} else {
		for ( int32 note : notes ) {
			if ( !_ForwardPlayer ) {
				notesFinal.Add( NotesSpectrumAvg[note] );
			} else {
				notesFinal.Add( NotesSpectrumAvgForward[note] );
			}
		}
	}
	return notesFinal;
}

TArray<float> UCGameMusicComp::GetNotesAmplitude( const TArray<int32> _NotesIndexs, const bool _ForwardPlayer, const bool _Validation ) {
	TArray<int32> notes = _NotesIndexs;
	TArray<float> notesFinal;
	const int32 lastIndex = NotesAmplitudeLast;
	if ( _Validation ) {
		for ( int32 note : notes ) {
			note = FMath::Min( note, lastIndex );
			if ( !_ForwardPlayer ) {
				notesFinal.Add( NotesAmplitude[note] );
			} else {
				notesFinal.Add( NotesAmplitudeForward[note] );
			}
		}
	} else {
		for ( int32 note : notes ) {
			if ( !_ForwardPlayer ) {
				notesFinal.Add( NotesAmplitude[note] );
			} else {
				notesFinal.Add( NotesAmplitudeForward[note] );
			}
		}
	}
	return notesFinal;
}

bool UCGameMusicComp::GetBeatStatus( const int32 _BeatIndex, const bool _ForwardPlayer, const bool _Validation ) {
	int32 beat = _BeatIndex;
	if ( _Validation ) { beat = FMath::Clamp( beat, 0, BEAT_NUM_NOTES-1 ); }
	if ( !_ForwardPlayer ) {
		return BeatsStatus[beat];
	} else {
		return BeatsStatusForward[beat];
	}
}

float UCGameMusicComp::GetBeatAmplitude( const int32 _BeatIndex, const bool _ForwardPlayer, const bool _Validation ) {
	int32 beat = _BeatIndex;
	if ( _Validation ) { beat = FMath::Clamp( beat, 0, BEAT_NUM_NOTES-1 ); }
	if ( !_ForwardPlayer ) {
		return BeatsAmplitude[beat];
	} else {
		return BeatsAmplitudeForward[beat];
	}
}

int32 UCGameMusicComp::GetBeatPM( const int32 _BeatIndex, const bool _ForwardPlayer, const bool _Validation ) {
	int32 beat = _BeatIndex;
	if ( _Validation ) { beat = FMath::Clamp( beat, 0, BEAT_NUM_NOTES - 1 ); }
	if ( !_ForwardPlayer ) {
		return BeatsPM[beat];
	} else {
		return BeatsPMForward[beat];
	}
}


//
// Volumes
//

float UCGameMusicComp::GetVolumeMaster() {
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			if ( lGI->GameSettings != nullptr ) {
				return lGI->GameSettings->AudioVolumeMaster;
			}
		}
	}
	return 0;
}

float UCGameMusicComp::GetVolumeMusic() {
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			if ( lGI->GameSettings != nullptr ) {
				return lGI->GameSettings->AudioVolumeMusic;
			}
		}
	}
	return 0;
}

float UCGameMusicComp::GetVolumeAnnouncer() {
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			if ( lGI->GameSettings != nullptr ) {
				return lGI->GameSettings->AudioVolumeAnnouncer;
			}
		}
	}
	return 0;
}

float UCGameMusicComp::GetVolumeSFX() {
	if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			if ( lGI->GameSettings != nullptr ) {
				return lGI->GameSettings->AudioVolumeSFX;
			}
		}
	}
	return 0;
}

void UCGameMusicComp::SetVolumeMaster( const float _Volume, const bool _UpdateSettings ) {
	if ( _UpdateSettings ) {
		if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
			UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
			if ( lGI != nullptr ) {
				if ( lGI->GameSettings != nullptr ) {
					lGI->GameSettings->AudioVolumeMaster = ( _Volume );
				}
			}
			// TODO SOUND CLASS CHANGE HERE
		}
	}
	if ( AudioPlayer ) {
		AudioPlayer->SetPlaybackVolume( GetVolumeMusic() * _Volume );
	}
	if ( MediaPlayer && MediaSoundComponent ) {
		MediaSoundComponent->SetVolumeMultiplier( GetVolumeMusic() * _Volume );
	}
	OnVolumeChanged.Broadcast();
}

void UCGameMusicComp::SetVolumeMusic( const float _Volume, const bool _UpdateSettings ) {
	if ( _UpdateSettings ) {
		if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
			UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
			if ( lGI != nullptr ) {
				if ( lGI->GameSettings != nullptr ) {
					lGI->GameSettings->AudioVolumeMusic = ( _Volume );
				}
			}
			// TODO SOUND CLASS CHANGE HERE
		}
	}
	if ( AudioPlayer ) {
		AudioPlayer->SetPlaybackVolume( GetVolumeMaster() * _Volume );
	}
	if ( MediaPlayer && MediaSoundComponent ) {
		MediaSoundComponent->SetVolumeMultiplier( GetVolumeMaster() * _Volume );
	}
	OnVolumeChanged.Broadcast();
}

void UCGameMusicComp::SetVolumeAnnouncer( const float _Volume, const bool _UpdateSettings ) {
	if ( _UpdateSettings ) {
		if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
			UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
			if ( lGI != nullptr ) {
				if ( lGI->GameSettings != nullptr ) {
					lGI->GameSettings->AudioVolumeAnnouncer = ( _Volume );
				}
			}
		}
	}
	OnVolumeChanged.Broadcast();
}

void UCGameMusicComp::SetVolumeSFX( const float _Volume, const bool _UpdateSettings ) {
	if ( _UpdateSettings ) {
		if ( GetWorld() != nullptr && GetWorld()->GetGameInstance() != nullptr ) {
			UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
			if ( lGI != nullptr ) {
				if ( lGI->GameSettings != nullptr ) {
					lGI->GameSettings->AudioVolumeSFX = ( _Volume );
				}
			}
		}
	}
	OnVolumeChanged.Broadcast();
}

//
// Vis frequencies
//


void UCGameMusicComp::SetFrequencyVisualMasterScale( const float _Value ) {
	float value = FMath::Clamp( _Value, 0.0f, 1.0f );
	FrequencyVisualMasterScale = value;

	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI != nullptr ) {
		lGI->GameSettings->HzVisualMaster = value;
	}
}

float UCGameMusicComp::GetFrequencyVisualMasterScale() {
	return FrequencyVisualMasterScale;
}

void UCGameMusicComp::SetFrequencyVisualScale( const int32 _NoteIndex, const float _Value, const bool _Half, const bool _Validation ) {
	int32 note = _NoteIndex;
	float value = FMath::Clamp( _Value, 0.0f, 1.0f );
	if( _Half ) {
		note = note * 2;
		int32 noteAlt = _NoteIndex;
		noteAlt = note + 1;
		if ( _Validation ) {
			note = FMath::Clamp( note, 0, 19 );
			noteAlt = FMath::Clamp( noteAlt, 0, 19 );
		}
		FrequencyVisualScales[note] = value;
		FrequencyVisualScales[noteAlt] = value;

		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			lGI->GameSettings->HzVisualValues[note] = value;
			lGI->GameSettings->HzVisualValues[noteAlt] = value;
		}
	} else {
		if ( _Validation ) {
			note = FMath::Clamp( note, 0, 19 );
		}

		FrequencyVisualScales[note] = value;

		UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
		if ( lGI != nullptr ) {
			lGI->GameSettings->HzVisualValues[note] = value;
		}
	}
}


float UCGameMusicComp::GetFrequencyVisualScale( const int32 _NoteIndex, const bool _Pure, const bool _Validation ) {
	int32 note = _NoteIndex;
	if ( _Validation ) { note = FMath::Clamp( note, 0, 19 ); }
	if ( _Pure ) {
		return FrequencyVisualScales[note];
	} else {
		return FrequencyVisualScales[note] * FrequencyVisualMasterScale;
	}
}

//
// Physical Frequencies
//

void UCGameMusicComp::SetFrequencyVibrationMasterScale( const float _Value ) {
	float value = FMath::Clamp( _Value, 0.0f, 1.0f );
	FrequencyVibrationMasterScale = value;

	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI != nullptr ) {
		lGI->GameSettings->ForceFeedbackShakeIntensity = value;
	}
}

float UCGameMusicComp::GetFrequencyVibrationMasterScale() {
	return FrequencyVibrationMasterScale;
}

void UCGameMusicComp::SetFrequencyVibrationScale( const int32 _HzIndex, const float _Value, const bool _Validation ) {
	int32 index = _HzIndex;
	float value = FMath::Clamp( _Value, 0.0f, 1.0f );

	if ( _Validation ) { index = FMath::Clamp( index, 0, FrequencyVibrationScales.Num() - 1 ); }
	FrequencyVibrationScales[index] = value;

	UCGameInstance* lGI = Cast<UCGameInstance>( GetWorld()->GetGameInstance() );
	if ( lGI != nullptr ) {
		lGI->GameSettings->ForceFeedbackShakeIntensitiesHz[index] = value;
	}

}

TArray<float> UCGameMusicComp::GetAllFrequenciesVibrationScale() {
	return FrequencyVibrationScales;
}

float UCGameMusicComp::GetFrequencyVibrationScaled( const int32 _HzIndex, const bool _Pure, const bool _Validation ) {
	int32 index = _HzIndex;
	if ( _Validation ) { index = FMath::Clamp( index, 0, FrequencyVibrationScales.Num() - 1 ); }
	//return BeatsAmplitude[index] * FrequencyVibrationScales[index];
	if ( _Pure ) {
		return FrequencyVisualScales[index];
	} else {
		return FrequencyVibrationScales[index] * FrequencyVibrationMasterScale;
	}
}

//
// GAME CONTENT
//

// Data to store highscore
int32 UCGameMusicComp::FileExtensionIndex( const FTrackStruct _Track ) {
	int32 value = -1;
	switch ( _Track.Format ) {
		case ETrackFormats::eOGG:
			value = 0;
			break;
		case ETrackFormats::eWAV:
			value = 1;
			break;
		case ETrackFormats::eMP3:
			value = 2;
			break;
		case ETrackFormats::eFLAC:
			value = 3;
			break;
		case ETrackFormats::eSTREAM:
			value = 4;
			break;
		case ETrackFormats::eSERROR:
			value = -1;
			break;
	}
	return value;
}


FString UCGameMusicComp::GetCurrentTrackUniqueID() {
	FString fileCode = "";
	if ( !UHEVLibraryIOParser::GetMD5HashToString( Track.URL, fileCode ) ) {
		return "";
	}
	fileCode = FString::FromInt(Track.Title.Len()) + ":" + fileCode  + ":" + FString::FromInt( FileExtensionIndex( Track ) ) + FString::FromInt( FMath::TruncToInt( Track.Length ));
	return fileCode;
}

bool UCGameMusicComp::GetValidatedTrack( const FString _File, FTrackStruct& _Track ) {
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::GetValidatedTrack()" ) );
	FString url = "";
	if ( !ValidateGetTrackURL( _File, url ) ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::GetValidatedTrack() Invalid File" ) );
		return false;
	}
	FTrackStruct track;
	track.URL = url;
	if ( !GetTrackMeta( track, track ) ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::GetValidatedTrack() Invalid Meta" ) );
		return false;
	}

	if ( !GetTrackUniqueID( track, track ) ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::GetValidatedTrack() Invalid Meta" ) );
	}
	UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::GetValidatedTrack() Validated" ) );
	_Track = track;
	return true;
}

bool UCGameMusicComp::GetTrackUniqueID( const FTrackStruct _File, FTrackStruct& _Track ) {
	FTrackStruct track = _File;
	FString code = "";
	if ( !UHEVLibraryIOParser::GetMD5HashToString( track.URL, code ) ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::GetTrackUniqueID() Invalid MD5" ) );
		return false;
	}
	track.ID = FString::FromInt( track.Title.Len() ) + FString::FromInt( FileExtensionIndex( track ) ) + ":" + code + ":" + FString::FromInt( FMath::TruncToInt( track.Length ) );
	_Track = track;
	return true;
}

bool UCGameMusicComp::ValidateTracksIDs( const FString _TrackID, const FString _ReferenceID, const bool _MustBeEqual ) {
	if ( _TrackID.Len() < 1 || _ReferenceID.Len() < 1 ) {
		UE_LOG( LogTemp, Warning, TEXT( "ValidateTracksIDs(): Invalid ID" ) );
		return false;
	}

	// Title Name Less important // md5 // Time
	TArray<FString> sourceTrackStr;
	FString trackID = _TrackID;
	trackID.ParseIntoArray( sourceTrackStr, TEXT( ":" ), true );

	FString referenceID = _ReferenceID;
	TArray<FString> sourceReferenceStr;
	referenceID.ParseIntoArray( sourceReferenceStr, TEXT( ":" ), true );

	if ( sourceTrackStr.Num() != 3 ) {
		UE_LOG( LogTemp, Warning, TEXT( "ValidateTracksIDs(): Invalid ID" ) );
		return false;
	}

	if ( _TrackID == _ReferenceID ) {
		UE_LOG( LogTemp, Warning, TEXT( "ValidateTracksIDs(): Complete Valid ID" ) );
		return true;
	}

	if ( _MustBeEqual ) {
		return false;
	}

	if ( sourceTrackStr[2] != sourceReferenceStr[2] ) {
		UE_LOG( LogTemp, Warning, TEXT( "ValidateTracksIDs(): Invalid Last Index" ) );
		return false;
	}
	bool state = false;
	if ( sourceTrackStr[1] == sourceReferenceStr[1] ) {
		state = true;
	}

	if ( sourceTrackStr[0] == sourceReferenceStr[0] ) {
		state = true;
	}

	if ( !state ) {
		UE_LOG( LogTemp, Warning, TEXT( "ValidateTracksIDs(): Invalid Middle Indexes" ) );
		return false;
	} else {
		UE_LOG( LogTemp, Warning, TEXT( "ValidateTracksIDs(): Half Valid ID" ) );
		return true;
	}
}

float UCGameMusicComp::GetGameTrackMeta( FString& _Title, FString& _Name, FString& _Genre, FString& _ID ) {
	_Title = TrackGame.Title;
	_Name = TrackGame.Name;
	_Genre = TrackGame.Genre;
	_ID = TrackGame.ID;
	return TrackGame.Length;
}

// Game Track set and loader
bool UCGameMusicComp::SetGameTrack( const FTrackStruct _Track ) {
	if ( ThreadLoading || ThreadLoadingTracks ) {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameMusicComp::SetGameTrack() Thread In Use" ) );
		return false;
	}

	AudioAnalyzerPlayerDecoder* audioDec = new AudioAnalyzerPlayerDecoder();
	FTrackStruct curSong = _Track;

	if ( audioDec->loadAudioInfo( curSong.URL ) ) {
		FString name, ext, extra, title, artist, album, year, genre;
		audioDec->getMetadata( name, ext, extra, title, artist, album, year, genre );
		curSong.Name = name;
		curSong.Title = title;
		curSong.Artist = artist;
		curSong.Album = album;
		curSong.Year = year;
		curSong.Genre = genre;
		curSong.ID = curSong.Name + "_" + curSong.Album;
		curSong.URL = curSong.URL;

		if ( curSong.Title == "" || curSong.Title == " " ) {
			curSong.Title = curSong.Name;
		}

		ext = FPaths::GetExtension( curSong.URL, true );
		if ( ext == ".ogg" ) {
			curSong.Format = ETrackFormats::eOGG;
		} else if ( ext == ".wav" ) {
			curSong.Format = ETrackFormats::eWAV;
		} else if ( ext == ".mp3" ) {
			curSong.Format = ETrackFormats::eMP3;
		} else if ( ext == ".flac" ) {
			curSong.Format = ETrackFormats::eFLAC;
		} else {
			curSong.Format = ETrackFormats::eSERROR;
		}
		if ( curSong.Format != ETrackFormats::eSERROR &&  curSong.Length > 14.f ) { // Safe lock here
			UE_LOG( LogTemp, Warning, TEXT( "SetGameTrack() Track Load" ) );
		} else {
			UE_LOG( LogTemp, Warning, TEXT( "SetGameTrack() Track Invalid at Load" ) );
			return false;
		}
	} else {
		UE_LOG( LogTemp, Warning, TEXT( "SetGameTrack() Track Invalid" ) );
		return false;
	}



	// Basic metadata
	Device = EMusicDeviceList::eSingle;
	TrackIndex = 0;
	PlaylistIndex = 0;
	PlaylistTracksNum = 1;
	PlaylistName = "GameTrack";
	ForwardPlayer = true;
	InGame = true;
	LastDevice = Device;
	TrackAutoPlayOnLoad = true;

	// Set track to load
	SetLoadTrackData( _Track );

	return true;
}
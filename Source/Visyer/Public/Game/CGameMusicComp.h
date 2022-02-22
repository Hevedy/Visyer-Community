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
#include "Components/AudioComponent.h"
#include "Components/ActorComponent.h"
#include "MediaSoundComponent.h"
#include "MediaPlayer.h"
#include "Misc/DateTime.h"
#include "AudioAnalyzerPlayerDecoder.h"
#include "AudioAnalyzerManager.h"
#include "VisyerTypes.h"
#include "HEVLibraryIO.h"
#include "HEVLibraryIOParser.h"
#include "VisyerTypes.h"
#include "CGameMusicComp.generated.h"

UENUM( BlueprintType )
enum class EPlayerState : uint8 {
	PLAY,
	PAUSE,
	STOP,
	FASTFORWARD,
	FASTBACKWARD,
	NEXT,
	PREV,
	NONE 
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FTrackStartedEvent );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FTrackEndedEvent );

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FTrackForwardStartedEvent );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FTrackForwardEndedEvent );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FTrackSetEvent, bool, _Valid );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FTrackUpdateEvent );

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FVolumeChangeEvent );

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FPreFilesReadEvent );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FPostFilesReadEvent );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FMPInitializedEvent );

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FThreadLoadPlaylistsLoadingEvent );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FThreadLoadTracksLoadingEvent );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FThreadLoadPlaylistsLoadedEvent );
DECLARE_DYNAMIC_MULTICAST_DELEGATE( FThreadLoadTracksLoadedEvent );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FBeatEvent, int32, _Index, float, _Amplitude, float, _Pitch );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FBeatForwardEvent, int32, _Index, float, _Amplitude, float, _Pitch );

DECLARE_DYNAMIC_DELEGATE( FThreadLoadedBP );

// Events for BP lists callers
UDELEGATE( BlueprintAuthorityOnly )
DECLARE_DYNAMIC_DELEGATE( FThreadLoadedDevice );

UDELEGATE( BlueprintAuthorityOnly )
DECLARE_DYNAMIC_DELEGATE( FThreadLoadedPlaylist );

UDELEGATE( BlueprintAuthorityOnly )
DECLARE_DYNAMIC_DELEGATE( FThreadLoadedTrack );

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FThreadEvent, bool, _Loading );

UCLASS( Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent, ClassGroup = Custom ) )
class VISYER_API UCGameMusicComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCGameMusicComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FMPInitializedEvent OnInitialized;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FPreFilesReadEvent OnFilesReadStart;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FPostFilesReadEvent OnFilesReadEnd;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FVolumeChangeEvent OnVolumeChanged;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FTrackStartedEvent OnTrackStarted;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FTrackEndedEvent OnTrackEnded;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FTrackForwardStartedEvent OnTrackForwardStarted;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FTrackForwardEndedEvent OnTrackForwardEnded;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FTrackSetEvent OnTrackSet;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FTrackUpdateEvent OnTrackUpdate;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FThreadLoadPlaylistsLoadingEvent OnThreadLoadingPlaylists;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FThreadLoadPlaylistsLoadedEvent OnThreadLoadedPlaylists;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FThreadLoadTracksLoadingEvent OnThreadLoadingTracks;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FThreadLoadTracksLoadedEvent OnThreadLoadedTracks;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FBeatEvent OnBeat;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FBeatForwardEvent OnBeatForward;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FThreadEvent OnThread;


	UFUNCTION( BlueprintPure, Category = "Class" )
		bool HasInitialized() { return Initialized; };

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "HevedyGame|General" )
		EGameModeList GameModeType = EGameModeList::eMainMenu;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "HevedyGame|General" )
		bool InGame;

	//
	// General
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "General" )
		bool Enabled;

	// Is Paused
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "General" )
		bool Paused;

	// Is Stop
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "General" )
		bool Stopped;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "General" )
		UAudioComponent* MusicSource;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "General" )
		UAudioComponent* AnnouncerSource;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "General" )
		UAudioComponent* PlayerSFXSource; // Basically not need as can play single time

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "General" )
		UMaterialParameterCollection* WorldParams;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UAudioAnalyzerManager* AudioPlayer;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UAudioAnalyzerManager* AudioPlayerForward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString AudioStreamChannel;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UMediaPlayer* MediaPlayer;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UMediaSoundComponent* MediaSoundComponent;

	//
	// Default Base Systems
	//

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioStartRadio;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioInsertCassete;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioInsertPurpleRay;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioPlayRadio;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioPlayCassete;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioPlayPurpleRay;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioPauseCassete;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioPausePurpleRay;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioStopRadio;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioStopCassete;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|SFX" )
		USoundCue* AudioStopPurpleRay;


	//
	// Device General
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		EMusicDeviceList Device = EMusicDeviceList::eCassette;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		EMusicDeviceList LastDevice = EMusicDeviceList::eCassette;


	//
	// Song General
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		int32 TrackIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		float TrackTime;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		float TrackForwardTime;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		float TrackLength;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		bool TrackLoopPlay = false;

	// Song Settings
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		float TrackVolume;
	
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		float TrackPitch;
	
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		float TrackBass;
	
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		float TrackEcho;

	// Song Info
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FString TrackName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FString TrackTitle;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FString TrackURL;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FString TrackArtist;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FTrackStruct Track;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FTrackStruct TrackGame;

	//
	// Playlist General
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|PlayList" )
		int32 PlaylistIndex = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|PlayList" )
		int32 PlaylistTracksNum = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|PlayList" )
		FString PlaylistName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|PlayList" )
		bool PlaylistAutoPlay = true;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|PlayList" )
		bool PlaylistRandomOrder = false;
	//
	// Radio General
	//
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Radio" )
		FString RadioName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Radio" )
		FString RadioURL;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Radio" )
		int32 RadioLastAdIndex;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Radio" )
		FDateTime LocalTime;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Radio" )
		FDateTime OnlineTime;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FRadioNetStationStruct RadioNet;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Player|Song" )
		FRadioLocalStationStruct RadioLocal;



	float RadioAdPerMusicVolume = 0.75f; // Audio Modification at play ad

	float RadioAdLerpTime = 3.0f; // Time before and at end of the add while audio changes

	bool RadioAdHour = true; // At the 10:00 o clock notice it

	float RadioAdLenght; // Get total Radio Lengt

	float RadioAdCount; // Count Audio

	// Radios LIST
	UFUNCTION( BlueprintPure, Category = "Thread" )
		bool GetPlaylistThreadData( int32& _Loaded, int32& _Total, float& _Percentage );

	UFUNCTION( BlueprintPure, Category = "Thread" )
		bool GetTrackThreadData( int32& _Loaded, int32& _Total, float& _Percentage );

	UFUNCTION()
		void DefaultSettings();

	UFUNCTION()
		void InitializeAudioPlayer();

	//
	// Functions
	//
	// Player Whole Setup
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Music Player" )
		bool Setup( const bool _InGame = false );

	virtual bool Setup_Implementation( const bool _InGame = false );

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Music Player" )
		void Reset();

		virtual void Reset_Implementation();

	UFUNCTION()
		TArray<FString> GetTierBlackListedResources();

	UFUNCTION()
		static bool ValidateGetTrackURL( const FString& _SongURL, FString& _SongURLFixed );

	UFUNCTION()
		static bool ValidateGetRadioURL( const FString& _SongURL, FString& _SongURLFixed );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool PushSingleData( const FString& _URL );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool TrySetSingle( const FString& _URL );

	UFUNCTION(BlueprintCallable, Category = "Music Player")
		bool TrySetListen();

	UFUNCTION()
		FString GetDeviceName();

	UFUNCTION()
		void GetSetInstanceDevicePlaylist();

	UFUNCTION()
		void SetInstanceDevicePlaylist();

	UFUNCTION()
		bool ReadFiles();

	UFUNCTION()
		bool ReadFilesSimple();

	UFUNCTION()
		void ReloadDevicePlaylists( const EMusicDeviceList _Device );

	UFUNCTION()
		void ReloadPlaylistTracks( const EMusicDeviceList _Device, const int32 _PlaylistIndex = 0, const bool _ForceReload = false );


	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void ReloadDevice( FThreadLoadedDevice _OnDeviceComplete, const EMusicDeviceList _Device );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void ReloadPlaylist( FThreadLoadedPlaylist _OnPlaylistComplete, const EMusicDeviceList _Device, const int32 _PlaylistIndex );

	UFUNCTION()
		void EmptyParams();

	UFUNCTION()
		void DefaultForwardParams();

	UFUNCTION()
		void PlayerParams();

	UFUNCTION()
		void ForwardPlayerParams();

	UFUNCTION()
		void UpdateParams();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void SetDevice( const EMusicDeviceList _Device = EMusicDeviceList::eCassette );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		EMusicDeviceList GetDevice();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool SetPlaylistIndex( FThreadLoadedBP _OnLoadComplete, const int32 _Index, const int32 _TrackIndex = 0, const bool _AutoPlay = false );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		int32 GetPlaylist( FString& _Name, int32& _TracksNum, int32& _TrackIndex );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool SetTrackIndex( const int32 _Index, const bool _AutoPlay = false );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool SetTrackRandomIndex( const bool _AutoPlay = false );

	// Player Play Start
		void PlayLogic();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void Play( bool _AutoManage = true );

		void PlayForwardLogic();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void PlayForward( bool _AutoManage = true );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void SetTime( const float _Time );
	
	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void FastForward();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void FastBackward();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void Rewind();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void SetTrackLoop( const bool _TrackLoop = true );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool GetTrackLoop();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void Pause();
	
	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void Stop();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void Next();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void Prev();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void NextPlaylist();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void PrevPlaylist();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void SetAutoPlay( const bool _AutoPlay = true );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool GetAutoPlay();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void SetRandomPlay( const bool _RandomPlay = true );

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		bool GetRandomPlay();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void OnTrackStart();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void OnTrackEnd();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void OnTrackForwardStart();

	UFUNCTION( BlueprintCallable, Category = "Music Player" )
		void OnTrackForwardEnd();

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		TArray<FTrackStruct> GetTracks( const TArray<FTrackStruct>& _TrackFiles, const int32 _Limit, const bool _Soft = false );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		static bool GetTrackMeta( const FTrackStruct _TrackFile, FTrackStruct& _Track );

	UFUNCTION( BlueprintPure, Category = "Player|Settings" )
		float GetVolumeMaster();

	UFUNCTION( BlueprintPure, Category = "Player|Settings" )
		float GetVolumeMusic();

	UFUNCTION( BlueprintPure, Category = "Player|Settings" )
		float GetVolumeAnnouncer();

	UFUNCTION( BlueprintPure, Category = "Player|Settings" )
		float GetVolumeSFX();

	UFUNCTION( BlueprintCallable, Category = "Player|Settings" )
		void SetVolumeMaster( const float _Volume, const bool _UpdateSettings = true );

	UFUNCTION( BlueprintCallable, Category = "Player|Settings" )
		void SetVolumeMusic( const float _Volume, const bool _UpdateSettings = true );
		
	UFUNCTION( BlueprintCallable, Category = "Player|Settings" )
		void SetVolumeAnnouncer( const float _Volume, const bool _UpdateSettings = true );

	UFUNCTION( BlueprintCallable, Category = "Player|Settings" )
		void SetVolumeSFX( const float _Volume, const bool _UpdateSettings = true );

	UFUNCTION()
		void ResetGlobalVariables();

	UFUNCTION()
		void SetLoadTrackData( const FTrackStruct _Track );

	UFUNCTION()
		void OnTrackLoaded();

	UFUNCTION()
		void OnTrackLoadFailed();

	UFUNCTION()
		void OnTrackFinished();

	UFUNCTION()
		void OnTrackForwardFinished();

	UFUNCTION()
		void SetLoadNetRadioData( const FRadioNetStationStruct _Radio );

	UFUNCTION()
		void OnRadioLoaded( FString OpenedUrl );

	UFUNCTION()
		void OnRadioLoadFailed( FString FailedUrl );

	UFUNCTION()
		void OnRadioFinished();

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		bool GetTrackValid() { return TrackValid; };

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		bool GetTrackPlaying() { return TrackPlaying; };

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		int32 GetDevicePlaylistNum( const EMusicDeviceList _Device );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		int32 GetPlaylistTracksNum( const EMusicDeviceList _Device, const int32 _PlaylistIndex );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		FPlaylistStruct GetPlaylistInfo( const EMusicDeviceList _Device, const int32 _PlaylistIndex );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		FRadioNetStationStruct GetRadioNetInfo( const int32 _PlaylistIndex );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		FRadioLocalStationStruct GetRadioLocalInfo( const int32 _PlaylistIndex );

	UFUNCTION( BlueprintPure, Category = "Audio|Player" )
		FTrackStruct GetTrackSetInfo( FString& _Name, FString& _Title, FString& _Artist, FString& _Album, float& _Length );

	UFUNCTION( BlueprintPure, Category = "Audio|Player" )
		float GetTrackTime( float& _Total, float& _Percentage );

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		FTrackStruct GetTrackInfo( const EMusicDeviceList _Device, const int32 _PlaylistIndex, const int32 _TrackIndex );

	UFUNCTION( BlueprintPure, Category = "Music Player|UI" )
		bool GetPlayButtonStatus();

	UFUNCTION( BlueprintPure, Category = "Music Player" )
		USoundCue* GetSoundEffect( EPlayerState _State );

	UFUNCTION()
		void PlaySoundEffect( EPlayerState _State );

	UFUNCTION()
		void PlayRadioAd();

	//
	// Data Output
	// 


	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		TArray<float> GetAllNotesSpectrum( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? NotesSpectrum : NotesSpectrumForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		TArray<float> GetAllNotesSpectrumAvg( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? NotesSpectrumAvg : NotesSpectrumAvgForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		TArray<float> GetNotesSpectrum( const TArray<int32> _NotesIndexs, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		TArray<float> GetNotesSpectrumAvg( const TArray<int32> _NotesIndexs, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetKeyNoteSpectrum( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? KeyNoteSpectrum : KeyNoteSpectrumForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetKeyNoteSpectrumAvg( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? KeyNoteSpectrumAvg : KeyNoteSpectrumAvgForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetNoteSpectrum( const int32 _NoteIndex, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetNoteSpectrumAvg( const int32 _NoteIndex, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		TArray<float> GetAllNotesAmplitude( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? NotesAmplitude : NotesAmplitudeForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		TArray<float> GetNotesAmplitude( const TArray<int32> _NotesIndexs, const bool _ForwardPlayer = false,  const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetKeyNoteAmplitude( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? KeyNoteAmplitude : KeyNoteAmplitudeForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetNoteAmplitude( const int32 _NoteIndex, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetKeyPitch( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? KeyPitch : KeyPitchForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetKeyBeats( const bool _ForwardPlayer = false ) { return !_ForwardPlayer ? Beats : BeatsForward; };

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		bool GetBeatStatus( const int32 _BeatIndex, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetBeatAmplitude( const int32 _BeatIndex, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		int32 GetBeatPM( const int32 _BeatIndex, const bool _ForwardPlayer = false, const bool _Validation = true );

	UFUNCTION( BlueprintCallable, Category = "Audio|Notes" )
		void SetFrequencyVisualMasterScale( const float _Value );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetFrequencyVisualMasterScale();

	UFUNCTION( BlueprintCallable, Category = "Audio|Notes" )
		void SetFrequencyVisualScale( const int32 _NoteIndex, const float _Value, const bool _Half = true, const bool _Validation = true );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetFrequencyVisualScale( const int32 _NoteIndex, const bool _Pure = false, const bool _Validation = true );

	UFUNCTION( BlueprintCallable, Category = "Audio|Notes" )
		void SetFrequencyVibrationMasterScale( const float _Value );

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetFrequencyVibrationMasterScale();

	UFUNCTION( BlueprintCallable, Category = "Audio|Notes" )
		void SetFrequencyVibrationScale( const int32 _HzIndex, const float _Value, const bool _Validation = true );
	
	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		TArray<float> GetAllFrequenciesVibrationScale();

	UFUNCTION( BlueprintPure, Category = "Audio|Notes" )
		float GetFrequencyVibrationScaled( const int32 _HzIndex, const bool _Pure, const bool _Validation = true );

	const int32 NotesSpectrumNum = SPECTRUM_NUM_NOTES; //At time
	const int32 NotesSpectrumLast = FMath::Max( 0, NotesSpectrumNum - 1 );
	const float NotesSpectrumScale = 1.0f;

	const int32 NotesAmplitudeNum = AMPLITUDE_NUM_NOTES; //Over time, middle is the center
	const int32 NotesAmplitudeLast = FMath::Max( 0, NotesAmplitudeNum - 1 );
	const float NotesAmplitudeScale = 1.0f;

	const float KeyNoteAmplitudeIndex = FMath::Max( 0, (int32)( NotesAmplitudeNum / 2 ) - 1 );

	UFUNCTION( BlueprintPure, Category = "Audio|IO" )
		static int32 FileExtensionIndex( const FTrackStruct _Track );

	UFUNCTION( BlueprintCallable, Category = "Audio|IO" )
		FString GetCurrentTrackUniqueID();

	UFUNCTION( BlueprintCallable, Category = "Audio|IO" )
		static bool GetValidatedTrack( const FString _File, FTrackStruct& _Track );

	UFUNCTION( BlueprintCallable, Category = "Audio|IO" )
		static bool GetTrackUniqueID( const FTrackStruct _File, FTrackStruct& _Track );

	UFUNCTION( BlueprintCallable, Category = "Audio|IO" )
		static bool ValidateTracksIDs( const FString _TrackID, const FString _ReferenceID, const bool _MustBeEqual = false );

	UFUNCTION( BlueprintCallable, Category = "Audio|IO" )
		float GetGameTrackMeta( FString& _Title, FString& _Name, FString& _Genre, FString& _ID );
	
	UFUNCTION( BlueprintCallable, Category = "Audio|IO" )
		bool SetGameTrack( const FTrackStruct _Track );

private:
	bool ThreadLoading = false;
	bool ThreadLoadingTrack = false;

	bool ThreadLoadingPlaylists = false;
	bool ThreadLoadingPlaylistsLast = false;
	int32 ThreadLoadedPlaylistsNum = 0; // Total Tracks Loaded
	int32 ThreadLoadingPlaylistsNum = 0; // Total Tracks To Load

	bool ThreadLoadingTracks = false;
	bool ThreadLoadingTracksLast = false;
	int32 ThreadLoadedTracksNum = 0; // Total Tracks Loaded
	int32 ThreadLoadingTracksNum = 0; // Total Tracks To Load

	//int32 ThreadLoadingPlayListStoredIndex = 0;

	TArray<FVector2D> LocalSpectrumHz;
	TArray<float> RadioSpectrumHz;

	TArray<float> NotesSpectrum;
	TArray<float> NotesSpectrumForward;
	TArray<float> NotesSpectrumAvg;
	TArray<float> NotesSpectrumAvgForward;
	TArray<float> NotesSpectrumDefault; //const 

	float KeyNoteSpectrum = 0.f;
	float KeyNoteSpectrumAvg = 0.f;
	float KeyNoteSpectrumForward = 0.f;
	float KeyNoteSpectrumAvgForward = 0.f;

	TArray<float> NotesAmplitude;
	TArray<float> NotesAmplitudeForward;
	TArray<float> NotesAmplitudeDefault; //Const

	float KeyNoteAmplitude = 0.f;
	float KeyNoteAmplitudeForward = 0.f;

	float KeyPitch = 0.f;
	float KeyPitchForward = 0.f;


	TArray<bool> BeatsStatus;
	TArray<bool> BeatsStatusForward;
	TArray<float> BeatsAmplitude;
	TArray<float> BeatsAmplitudeForward;
	TArray<int32> BeatsPM;
	TArray<int32> BeatsPMForward;
	TArray<bool> BeatsStatusDefault;
	TArray<float> BeatsAmplitudeDefault;
	TArray<int32> BeatsPMDefault;
	int32 Beats = 0;
	int32 BeatsForward = 0;

	TArray<float> FrequencyVisualScales;
	TArray<float> FrequencyVisualScalesDefault;

	float FrequencyVisualMasterScale = 1.f;


	TArray<float> FrequencyVibrationScales;
	TArray<float> FrequencyVibrationScalesDefault;
	float FrequencyVibrationMasterScale = 1.f;


	bool TrackIsReady = false;

	bool TrackAutoPlayOnLoad = false;


	// Forward player

	bool ForwardPlayer = false;

	float ForwardPlayerTime = 5.0f;


	UPROPERTY()
		bool Initialized = false;

	UPROPERTY()
		bool TrackPlaying = false;

	UPROPERTY()
		bool TrackForwardPlaying = false;

	UPROPERTY()
		bool LastTrackPlaying = false;

	UPROPERTY()
		bool TrackValid = false;

	UPROPERTY()
		bool TrackStarted = false;

	UPROPERTY()
		bool TrackEnded = false;

	UPROPERTY()
		bool TrackForwardStarted = false;

	UPROPERTY()
		bool TrackForwardEnded = false;


	UFUNCTION()
		bool CheckTrackStatus();


	TArray<float> NullDummyFloat;
	TArray<int32> NullDummyInt;
};

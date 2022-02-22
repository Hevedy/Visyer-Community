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
//#include "Components/ActorComponent.h"
#include "UObject/Object.h"
#include "GameFramework/GameUserSettings.h"
//#include "CGameInstance.h"
#include "HEVLibrarySettings.h"
#include "DiscordBlueprint.h"
//#include "DiscordHelper.h"
//#include "TwitchObject.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "CGameSocial.generated.h"

//Fix for circular dependency
class UCGameInstance;
// TODO
// Accesibility Settings
// Deaf People Pulses strike from music suits and gamepad
// 

// 

USTRUCT(BlueprintType)
struct FLeaderboardData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
		int32 Rank;

	UPROPERTY(BlueprintReadWrite)
		FString Score;

	FORCEINLINE bool operator==(const FLeaderboardData& A) const
	{
		return Rank == A.Rank;
	}
};
UENUM(BlueprintType)
enum class ELeaderboardType : uint8
{
	LT_None,
	LT_World,
	LT_Friends
};


// Change this to normal object and store it inside Instance with a value
//UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) ) //By default

//UCLASS( Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent, ClassGroup = Custom ) )
UCLASS( Blueprintable, BlueprintType )
class VISYER_API UCGameSocial : public UObject
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCGameSocial();

	~UCGameSocial();

	void VirtualBeginPlay();

	//void VirtualTickObject();

	void VirtualEndPlay();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Social" )
		void SetDiscordMusicInfo( const FString _DeviceName, const FString _PlayListName, const FString _SongName, const FString _ArtistName );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Social" )
		void SetDiscordGameInfo( const FString _Mode, const FString _Level, const FString _Players = "0" );

protected:
	// Called when the game starts
	//virtual void BeginPlay() override;

private:
	bool Tick( float DeltaSeconds );

	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;

	/** Handle to various registered delegates */
	FDelegateHandle TickDelegateHandle;


	UPROPERTY()
		UCGameInstance* GameInstance;

	//UDiscordRpc* DiscordRPC;

	//FDiscordRichPresence DiscordPresence;

	FString CurrentLevel;

	FString CurrentDevice;

	FString CurrentPlayList;

	// Estoy tendria que estar en gamemode
	//UTwitchObject* Twitch;

	UFUNCTION()
		void DiscordConneted();

	UFUNCTION()
		void DiscordDisconnected();

	UFUNCTION()
		void DiscordError();

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// This is the actual array of settings
	//TArray<FHEVSettingMenuStruct*> SettingsArray;

	virtual void BeginDestroy() override;

	void DefaultSettings();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Hevedy|Game|Social" )
		bool Setup( UCGameInstance* _Instance );

		virtual bool Setup_Implementation( UCGameInstance* _Instance );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Social" )
		bool ReadConfig();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Social" )
		bool WriteConfig();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Social" )
		bool DiscordConnect();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Social" )
		void DiscordDisconnect();

	UFUNCTION( BlueprintCallable, Category = "Hevedy|Game|Social" )
		bool UpdatePresence();

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		bool LastReadFailed;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		bool LastWriteFailed;

	//
	// Social Media
	//
	// Do this better with the pre made C# app
	// Don't use Twitch or youtube but rather Discord to control the voting

	// Game This Nexus
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString GameThisName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString GameThisNexusID;

	// Twitch Individual
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString TwitchChannel;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString TwitchName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString TwitchBot;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		bool DiscordEnabled = false;

	// Discord Individual
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString DiscordChannel;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString DiscordName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString DiscordBotName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		bool SteamEnabled = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Hevedy|Game|Social" )
		FString SteamName;



	///
	/// LEADERBOARDS
	///
	/// 


	UFUNCTION(BlueprintCallable, Category = "Hevedy|Game|Social")
	void BootLeaderBoard();

	UFUNCTION(BlueprintCallable, Category = "Hevedy|Game|Social")
	int GetLeaderBoardScore(FName _User, FString _DataTable);

	UFUNCTION(BlueprintCallable, Category = "Hevedy|Game|Social")
	void SetLeaderBoardScore(FName _User, FString _DataTable, int _Score);

};


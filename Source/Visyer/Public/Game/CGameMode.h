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
#include "GameFramework/GameMode.h"
#include "CGameSettings.h"
#include "CGameMusicComp.h"
#include "CGamePianoComp.h"
#include "CGameTwitchComp.h"
#include "CGameDiscordComp.h"
#include "MediaSoundComponent.h"
#include "CGameMode.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGMInitializedEvent );
/**
 * 
 */
UCLASS()
class VISYER_API ACGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACGameMode( const FObjectInitializer& ObjectInitializer );

	// returns default pawn class for given controller
	virtual UClass* GetDefaultPawnClassForController_Implementation( AController* InController ) override;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = GameMode )
		TSubclassOf<APawn> DefaultSpawnPawnClass;

protected:
	UPROPERTY( BlueprintReadWrite, Category = "ClassManager" )
		bool bInitialized = false;

public:
	UFUNCTION( BlueprintCallable, Category = "ClassManager" )
	virtual void ConstructorDefaults();

	virtual void BeginPlay() override;

	virtual void PreInitializeComponents() override;

	virtual void PostInitializeComponents() override;

	UFUNCTION( BlueprintCallable, Category = "ClassManager" )
	virtual void Initialize();

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, Category = "ClassManager" )
		void OnInitialize();

	UFUNCTION( BlueprintPure, Category = "ClassManager" )
		bool GetInitialized() const;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FGMInitializedEvent OnInitialized;

	virtual void Reset() override;

	UFUNCTION( BlueprintCallable, Category = "ClassManager" )
	virtual void ResetDefaults();

	virtual void Destroyed() override;

	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION()
		void DefaultSettings();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Music Player" )
		void Setup();

	virtual void Setup_Implementation();

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Setup" )
		TSubclassOf<UCGameMusicComp> MusicComponentClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UCGameMusicComp* MusicComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Setup" )
		TSubclassOf<UCGamePianoComp> PianoComponentClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UCGamePianoComp* PianoComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UMediaSoundComponent* MediaSoundComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Setup" )
		TSubclassOf<UMaterialParameterCollection> WorldParamsClass;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UCGameTwitchComp* TwitchComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
		UCGameDiscordComp* DiscordComponent;

private:

};

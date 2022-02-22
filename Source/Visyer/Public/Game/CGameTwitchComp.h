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
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
//#include "CGameSettings.h"
#include "TwitchActorComponent.h"
//#include "TwitchStructs.h"
#include "CGameTwitchComp.generated.h"


UCLASS( Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent, ClassGroup = Custom ) )
class VISYER_API UCGameTwitchComp : public UTwitchActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCGameTwitchComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//
	// Functions
	//
	UFUNCTION()
		void DefaultSettings();
	// Player Whole Setup
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable )
		bool Setup();

	virtual bool Setup_Implementation();

	UFUNCTION()
		void Connect();

	UFUNCTION()
		void StartVote( FString _Reason, TArray<FString> _VoteStrings, float _Time = 0.f);

	UFUNCTION()
		void EndVote();

	UFUNCTION()
		float GetRemainVoteTime();

	UFUNCTION()
		TArray<int32> GetVoteCount();

private:
	UPROPERTY()
		UTwitchActorComponent* TwitchComponent;

	UFUNCTION()
		void ChatRead( FString _Message, FTwitchChatMessageData _Data );

	UFUNCTION()
		void ChatWrite( FString _Message );
	//FChatMessageReceived, FString, message, FChatMessageData, data);


	FString AdminChannelName; // User channel name
	FString AdminName; // Actual user name to check admin
	TArray<FString> ModNames;
	bool HasMods = false;
	FString BotName;
	FString BotToken;

	bool VoteActive = false;
	float VoteTime = 0.f;
	FTimerHandle Vote_timer_;
	FString VoteReason;
	FString VoteStartCommand = "!";
	TArray<FString> VoteValidStrings;
	TArray<int32> VotedIndexs;
	TMap<FString, int32> Voters;

};

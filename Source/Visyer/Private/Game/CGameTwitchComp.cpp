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

#include "CGameTwitchComp.h"


// Sets default values for this component's properties
UCGameTwitchComp::UCGameTwitchComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}



// Called when the game starts
void UCGameTwitchComp::BeginPlay()
{
	Super::BeginPlay();
	// ...
	this->OnChatMessage.AddDynamic( this, &UCGameTwitchComp::ChatRead );
}

void UCGameTwitchComp::EndPlay( const EEndPlayReason::Type EndPlayReason ) {
	if ( TwitchComponent != nullptr ) {
		TwitchComponent->disconnectFromTwitch();
	}
	if ( Vote_timer_.IsValid() ) {
		GetWorld()->GetTimerManager().ClearTimer( this->Vote_timer_ );
	}
	Super::EndPlay( EndPlayReason );
}

// Called every frame
void UCGameTwitchComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCGameTwitchComp::DefaultSettings() {

}

bool UCGameTwitchComp::Setup_Implementation() {

	if ( GetWorld() != nullptr ) {
		Connect();
		return true;
	} else {
		return false;
	}
}


void UCGameTwitchComp::Connect() {
	if ( TwitchComponent != nullptr ) {
		TwitchComponent->setToken( "" );
		TwitchComponent->setChannel( "" );
		TwitchComponent->setUsername( "" ); //Bot Name
		TwitchComponent->connectToTwitch();
	}
}

void UCGameTwitchComp::ChatRead( FString _Message, FTwitchChatMessageData _Data ) {
	if ( _Message == "" || _Message == " " ) { return;  }
	FString msg = _Message;
	FString user = _Data.sender_username;
	if ( user == AdminName ) {
		if ( msg == ( VoteStartCommand + "StartVote" ) || msg == ( VoteStartCommand + "VoteStart" ) ) {
			if ( !VoteActive ) {
				//StartVote( FString _Reason, TArray<FString> _VoteStrings, float _Time )
				// Call Gamemode better
				return;
			}
		}
		// Admin
	} else {
		if ( HasMods ) {
			for ( FString& name : ModNames ) {
				if ( user == name ) {
					if ( msg == ( VoteStartCommand + "StartVote" ) || msg == ( VoteStartCommand + "VoteStart" ) ) {
						if ( !VoteActive ) {
							//StartVote( FString _Reason, TArray<FString> _VoteStrings, float _Time )
							// Call Gamemode better
							return;
						}
					}
					return;
				}
			}
		}

	}

	//int32 vIndex = VoteValidStrings.Find( msg );
	//if ( int32* uIndex = Voters.Find( user ) ) {


	if ( VoteActive ) {
		int32 vIndex = VoteValidStrings.Find( msg );
		if ( vIndex != INDEX_NONE ) {
			int32* uVote = Voters.Find( user );
			if ( uVote != nullptr ) {
				if ( *uVote != vIndex ) {
					VotedIndexs[*uVote]--;
					*uVote = vIndex;
					VotedIndexs[vIndex]++;
				}
				//(FString, int32) uV = Voters[uIndex];
			} else {
				Voters.Add( user, vIndex );
				VotedIndexs[vIndex]++;
			}
		}
	}
}

void UCGameTwitchComp::ChatWrite( FString _Message ) {
	if ( TwitchComponent != nullptr ) {
		if ( !TwitchComponent->bIsConnected ) { return; }
		TwitchComponent->sendMessage( _Message );
	}
}

void UCGameTwitchComp::StartVote( FString _Reason, TArray<FString> _VoteStrings, float _Time ) {
	if ( TwitchComponent == nullptr ) { return; }
	if ( !TwitchComponent->bIsConnected ) { return; }

	VoteValidStrings.Empty();
	VotedIndexs.Empty();
	Voters.Empty();

	VoteReason = _Reason;
	for( FString& str : _VoteStrings ) {
		VoteValidStrings.Add( VoteStartCommand + str );
	}

	VotedIndexs.Init( 0, FMath::Max( 0, VoteValidStrings.Num() - 1 ) );

	if ( _Time != 0.f ) {
		VoteTime = FMath::Max( 0.f, _Time );
		if ( GetWorld() != nullptr ) {
			GetWorld()->GetTimerManager().ClearTimer( this->Vote_timer_ );
			GetWorld()->GetTimerManager().SetTimer( this->Vote_timer_, this, &UCGameTwitchComp::EndVote, VoteTime, false );
		}
	} else {
		VoteTime = 0.f;
	}
	VoteActive = true;
}

void UCGameTwitchComp::EndVote() {
	VoteActive = false;
	VoteTime = 0.f;
}



float UCGameTwitchComp::GetRemainVoteTime() {
	if ( VoteActive && Vote_timer_.IsValid() ) {
		return GetWorld()->GetTimerManager().GetTimerRemaining( this->Vote_timer_ );
	} else {
		return VoteTime;
	}

}

TArray<int32> UCGameTwitchComp::GetVoteCount() {
	return VotedIndexs;
}
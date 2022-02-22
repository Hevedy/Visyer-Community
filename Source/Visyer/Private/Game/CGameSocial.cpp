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

#include "CGameSocial.h"
#include "CGameInstance.h"
#include "CMainBlueprintFL.h"


// Sets default values for this component's properties
UCGameSocial::UCGameSocial()
{

}

UCGameSocial::~UCGameSocial() {

}

void UCGameSocial::VirtualBeginPlay() {
	DiscordConnect();
	if ( DiscordEnabled ) {

	}
	if ( GetWorld() ) {

	}
	// If you want to keep this handle around then declare it in your header
	TickDelegate = FTickerDelegate::CreateUObject( this, &UCGameSocial::Tick );
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker( TickDelegate, 0.1f ); //1.0f
}

bool UCGameSocial::Tick( float DeltaSeconds ) {
	if ( DiscordEnabled ) {
		UDiscordRpc::RunCallbacks();
	}
	return true;
}

void UCGameSocial::VirtualEndPlay() {
	// Unregister ticker delegate
	FTicker::GetCoreTicker().RemoveTicker( TickDelegateHandle );
	if ( DiscordEnabled ) {
		UDiscordRpc::Shutdown();
		//UDiscordRpc::VirtualEndPlay();
	}
	DiscordDisconnect();
}

void UCGameSocial::BeginDestroy() {
	VirtualEndPlay();

	Super::BeginDestroy();
}

void UCGameSocial::DefaultSettings() {

}

bool UCGameSocial::Setup_Implementation( UCGameInstance* _Instance ) {
	GameInstance = _Instance;
	VirtualBeginPlay();
	return true;

}


bool UCGameSocial::ReadConfig() {
		UE_LOG( LogTemp, Warning, TEXT( "UCGameSocial::ReadConfig()" ) );
		if ( UHEVLibrarySettings::SettingsDirectoryValidate() ) {
			UE_LOG( LogTemp, Warning, TEXT( "true" ) );
			return true;
		} else {
			UE_LOG( LogTemp, Warning, TEXT( "false" ) );
			return false;
		}
}

bool UCGameSocial::WriteConfig() {
	return false;
}

bool UCGameSocial::DiscordConnect() {
	bool discordEnabled = false;
	if ( GameInstance != nullptr ) {
		discordEnabled = GameInstance->GetDiscordRPEnabled();
		DiscordEnabled = discordEnabled;
	}
	if ( DiscordEnabled  ) {
		UDiscordRpc::Initialize(0000, false );
	}
	return false;
}

void UCGameSocial::DiscordDisconnect() {
	UDiscordRpc::Shutdown();
}


void UCGameSocial::DiscordConneted() {

}

void UCGameSocial::DiscordDisconnected() {

}

void UCGameSocial::DiscordError() {

}

void UCGameSocial::SetDiscordMusicInfo( const FString _DeviceName, const FString _PlayListName, const FString _SongName, const FString _ArtistName ) {
	if ( DiscordEnabled ) {
		UDiscordRpc::UpdatePlayActivity( _SongName + " by " + _ArtistName, _DeviceName + " - " + _PlayListName );
	}

}

void UCGameSocial::SetDiscordGameInfo( const FString _Mode, const FString _Level, const FString _Players ) {
}

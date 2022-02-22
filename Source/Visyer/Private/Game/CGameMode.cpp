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

#include "CGameMode.h"
#include "CMainBlueprintFL.h"


ACGameMode::ACGameMode( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer ) {
	bInitialized = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bAllowTickBeforeBeginPlay = false;

	// Only call this on children
	//ConstructorDefaults();

}

void ACGameMode::ConstructorDefaults() {

}

void ACGameMode::BeginPlay() {
	Super::BeginPlay();

	Setup();
}

void ACGameMode::PreInitializeComponents() {
	Super::PreInitializeComponents();

	//MediaPlayer = NewObject<UMediaPlayer>( this, UMediaPlayer::StaticClass() );

	MediaSoundComponent = FindComponentByClass<UMediaSoundComponent>();

	// Runtime solution to add a component based in a proviced class from BP
	if ( MusicComponentClass->IsChildOf( UCGameMusicComp::StaticClass() ) && MusicComponentClass.GetDefaultObject() ) {
		MusicComponent = NewObject<UCGameMusicComp>( this, MusicComponentClass );
		MusicComponent->RegisterComponent();
	}

	if ( PianoComponentClass->IsChildOf( UCGamePianoComp::StaticClass() ) && PianoComponentClass.GetDefaultObject() ) {
		PianoComponent = NewObject<UCGamePianoComp>( this, PianoComponentClass );
		PianoComponent->RegisterComponent();
	}

	TwitchComponent = NewObject<UCGameTwitchComp>( this, UCGameTwitchComp::StaticClass() );
	TwitchComponent->RegisterComponent();

	DiscordComponent = NewObject<UCGameDiscordComp>( this, UCGameDiscordComp::StaticClass() );
	DiscordComponent->RegisterComponent();
}

void ACGameMode::PostInitializeComponents() {
	Super::PostInitializeComponents();

}

void ACGameMode::Initialize() {
	if ( bInitialized == true ) {
		return;
	}
	bInitialized = true;
	// Only call this on children

}

bool ACGameMode::GetInitialized() const {
	return bInitialized;
}

void ACGameMode::Reset() {
	Super::Reset();

}

void ACGameMode::ResetDefaults() {

}

void ACGameMode::Destroyed() {
	Super::Destroyed();
}

void ACGameMode::Tick( float DeltaSeconds ) {
	Super::Tick( DeltaSeconds );

}

void ACGameMode::DefaultSettings() {
	UE_LOG( LogTemp, Warning, TEXT( "Gamemode Settings: " ) );
	if ( MusicComponent != nullptr ) {

	}
	if ( PianoComponent != nullptr ) {
		PianoComponent->DefaultSettings();
	}

	if ( TwitchComponent != nullptr ) {
		TwitchComponent->DefaultSettings();
	}

	if ( DiscordComponent != nullptr ) {
		DiscordComponent->DefaultSettings();
	}

}

void ACGameMode::Setup_Implementation() {
	//Log Grey, Warning Yellow, Error Red
	UE_LOG( LogTemp, Warning, TEXT( "Gamemode Setup:" ) );

	if ( MusicComponent != nullptr ) { 

	}
	if ( PianoComponent != nullptr ) {
		PianoComponent->Setup();
	}
	if ( TwitchComponent != nullptr ) {
		TwitchComponent->Setup();
	}
	if ( DiscordComponent != nullptr ) {
		DiscordComponent->Setup();
	}

	DefaultSettings();
}

UClass* ACGameMode::GetDefaultPawnClassForController_Implementation( AController* InController ) {

	if ( DefaultSpawnPawnClass ) {
		return DefaultSpawnPawnClass;
	} else {
		return Super::GetDefaultPawnClassForController_Implementation( InController );
	}

}
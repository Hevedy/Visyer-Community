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

#include "CGameState.h"


ACGameState::ACGameState( const FObjectInitializer& ObjectInitializer ) : Super( ObjectInitializer ) {
	bInitialized = false;
	ConstructorDefaults();
	ResetDefaults();
}

void ACGameState::ConstructorDefaults() {

}

void ACGameState::BeginPlay() {
	Super::BeginPlay();
}

void ACGameState::PostInitializeComponents() {
	Super::PostInitializeComponents();
	Initialize();
}

void ACGameState::Initialize() {
	if ( bInitialized == true ) {
		return;
	}
	bInitialized = true;
	// CPP Code
	OnInitialize();
}

bool ACGameState::GetInitialized() const {
	return bInitialized;
}

void ACGameState::Reset() {
	Super::Reset();
	ResetDefaults();
}

void ACGameState::ResetDefaults() {

}

void ACGameState::Destroyed() {
	Super::Destroyed();
}


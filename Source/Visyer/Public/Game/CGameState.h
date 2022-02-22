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
#include "GameFramework/GameState.h"
#include "CGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FInitializedEventGS );
/**
 * 
 */
UCLASS()
class VISYER_API ACGameState : public AGameStateBase {
	GENERATED_BODY()



		ACGameState( const FObjectInitializer& ObjectInitializer );

protected:
	UPROPERTY( BlueprintReadWrite, Category = "ClassManager" )
		bool bInitialized;

public:
	UFUNCTION( BlueprintCallable, Category = "ClassManager" )
		void ConstructorDefaults();

	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	UFUNCTION( BlueprintCallable, Category = "ClassManager" )
		void Initialize();

	UFUNCTION( BlueprintImplementableEvent, Category = "ClassManager" )
		void OnInitialize();

	UFUNCTION( BlueprintPure, Category = "ClassManager" )
		bool GetInitialized() const;

	UPROPERTY( BlueprintAssignable, Category = "ClassManager" )
		FInitializedEventGS OnInitialized;

	virtual void Reset() override;

	UFUNCTION( BlueprintCallable, Category = "ClassManager" )
		void ResetDefaults();

	virtual void Destroyed() override;
};
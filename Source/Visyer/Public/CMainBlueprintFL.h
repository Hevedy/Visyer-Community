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
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VisyerTypes.h"
#include "Components/SplineComponent.h"
#include "CMainBlueprintFL.generated.h"


UENUM( BlueprintType )
enum class EEventMedalsList : uint8 {
	EBronze			UMETA( DisplayName = "Bronze" ),
	ESilver			UMETA( DisplayName = "Silver" ),
	EGold			UMETA( DisplayName = "Gold" ), // ...
	EPlatinum		UMETA( DisplayName = "Platinum" ), // Platinum time
	ECreator		UMETA( DisplayName = "Creator" ), // Creator Time
	EPersonal		UMETA( DisplayName = "Personal" ) // Personal record if is over any other medal or under any other medal
};

/**
 * 
 */
UCLASS()
class VISYER_API UCMainBlueprintFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION( BlueprintCallable, Category = "Hevedy|IO" )
		static bool OverwritePlaylistFile( const EMusicDeviceList _Device, const FString _FileName, const TArray<FString> _Lines );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|IO" )
		static bool DeletePlaylistFile( const EMusicDeviceList _Device, const FString _FileName );

	UFUNCTION( BlueprintCallable, Category = "Hevedy|IO" )
		static bool GetDirectoryContent( const FString _Directory, const TArray<FString> _Extensions, TArray<FString>& _Files, TArray<FString>& _Directories, 
										 TArray<FString>& _Mixed, TArray<FString>& _FilesUrls, TArray<FString>& _DirectoriesUrls, TArray<FString>& _MixedUrls );
	
	UFUNCTION( BlueprintPure, Category = "HevedyLib|IO|Helper" )
		static FString GameThisDir( const bool _Relative = false );

	UFUNCTION( BlueprintPure, Category = "HevedyLib|IO|Helper" )
		static FString MusicToPlaylistDir( const bool _Relative = false );

};

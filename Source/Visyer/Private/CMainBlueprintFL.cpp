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

#include "CMainBlueprintFL.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "GameFramework/Actor.h"
#include "Player/CHUD.h"
#include "Player/CPlayerController.h"
#include "Player/CPlayerState.h"
#include "Game/CGameMode.h"
#include "Game/CGameState.h"
#include "HEVLibraryMath.h"
#include "HEVLibraryIO.h"
#include "HEVLibraryIOParser.h"

#ifndef FNAME_TO_NUM
#define FNAME_TO_NUM( X ) const FName(TEXT(X)).GetNumber()
#endif


// Make sure update lists after this
bool UCMainBlueprintFL::OverwritePlaylistFile( const EMusicDeviceList _Device, const FString _FileName, const TArray<FString> _Lines ) {
	const FString resourcesRootDir = UHEVLibraryIO::GameResourcesDir( false );
	FString deviceName = "";
	FString url = _FileName;
	switch ( _Device ) {
		case  EMusicDeviceList::eSingle:
			deviceName = "Single";
			url = resourcesRootDir + "" + url;
			break;
		case  EMusicDeviceList::eCassette:
			deviceName = "Cassette";
			url = resourcesRootDir + "Cassettes/" + url;
			break;
		case  EMusicDeviceList::ePurpleRay:
			deviceName = "PurpleRay";
			url = resourcesRootDir + "PurpleRays/" + url;
			break;
		case  EMusicDeviceList::eRadioLocal:
			deviceName = "Local Radio";
			url = resourcesRootDir + "Radios/Local/" + url;
			break;
		case  EMusicDeviceList::eRadioNet:
			deviceName = "Online Radio";
			url = resourcesRootDir + "Radios/Online/" + url;
			break;
		default:
			return false;
			break;
	}
	url = url + ".vpl";


	// Remove duplicates
	TArray<FString> lines = UHEVLibraryIOParser::DeleteArrayDuplicates( _Lines, {}, { ".ogg", ".wav", ".mp3", ".flac" } );

	TArray<FString> linesFixed;
	linesFixed.Add( "; Generated file for " + UHEVLibraryIO::GetProjectName() + " - " + UHEVLibraryIO::GetProjectVersion() );
	linesFixed.Add( "; Device: " + deviceName );
	linesFixed.Add( "" );
	for ( FString line : lines ) {
		linesFixed.Add( line );
	}
	return UHEVLibraryIOParser::SaveStringArrayToFile( FPaths::GetPath( *url ), FPaths::GetCleanFilename( *url ), linesFixed, true );
}

bool UCMainBlueprintFL::DeletePlaylistFile( const EMusicDeviceList _Device, const FString _FileName ) {
	FString url = _FileName;
	const FString resourcesRootDir = UHEVLibraryIO::GameResourcesDir( false );
	switch ( _Device ) {
		case  EMusicDeviceList::eSingle:
			url = resourcesRootDir + "" + url;
			break;
		case  EMusicDeviceList::eCassette:
			url = resourcesRootDir + "Cassettes/" + url;
			break;
		case  EMusicDeviceList::ePurpleRay:
			url = resourcesRootDir + "PurpleRays/" + url;
			break;
		case  EMusicDeviceList::eRadioLocal:
			url = resourcesRootDir + "Radios/Local/" + url;
			break;
		case  EMusicDeviceList::eRadioNet:
			url = resourcesRootDir + "Radios/Online/" + url;
			break;
		default:
			break;
	}
	url = url + ".vpl";


	return UHEVLibraryIO::FileDelete( url );
}

bool UCMainBlueprintFL::GetDirectoryContent( const FString _Directory, const TArray<FString> _Extensions, TArray<FString>& _Files, TArray<FString>& _Directories, TArray<FString>& _Mixed, TArray<FString>& _FilesUrls, TArray<FString>& _DirectoriesUrls, TArray<FString>& _MixedUrls ) {
	TArray<FString> rawFiles;
	
	bool status = true;
	if ( !UHEVLibraryIO::GetDirectoriesFiles( rawFiles, _Directory, false, "*.*", false, false, false ) ) {
		status = false;
	} else {
		status = true;
	}

	UE_LOG( LogTemp, Warning, TEXT( "Raw Files Found: %d" ), rawFiles.Num() );

	TArray<FString> validExts;
	if ( _Extensions.Num() < 1 ) {
		validExts.Add( "*.*" );
	} else {
		validExts = _Extensions;
	}

	TArray<FString> filesNames;
	TArray<FString> filteredFiles;
	for ( const FString& file : rawFiles ) {
		if ( UHEVLibraryIO::FileExistsIsValidExt( file, validExts ) ) {
			filteredFiles.Add( file );
			filesNames.Add( FPaths::GetCleanFilename( file ) );
		}
	}

	TArray<FString> rawDirs;

	if ( !UHEVLibraryIO::GetSubDirectories( _Directory, rawDirs ) ) {
		if ( !status ) {
			return false;
		}
	} else {
		status = true;
	}

	TArray<FString> dirsNames;
	TArray<FString> filteredDirs;
	for ( const FString& folder : rawDirs ) {
		filteredDirs.Add( folder );
		FString str;
		if ( UHEVLibraryIO::DirectoryGetName( folder, str ) ) {
			dirsNames.Add( str );
		}

	}

	UE_LOG( LogTemp, Warning, TEXT( "Files Found: %d" ), rawFiles.Num() );
	UE_LOG( LogTemp, Warning, TEXT( "Folders Found: %d" ), rawDirs.Num() );

	_Files = filesNames;
	_Directories = dirsNames;
	_Mixed = filesNames;
	_Mixed.Append( dirsNames );

	_FilesUrls = filteredFiles;
	_DirectoriesUrls = filteredDirs;
	_MixedUrls = filteredFiles;
	_MixedUrls.Append( filteredDirs );
	return ( filteredFiles.Num() > 0 || filteredDirs.Num() > 0);

}


FString UCMainBlueprintFL::GameThisDir( const bool _Relative ) {
	return UHEVLibraryIO::UnpackDir( _Relative ) + "GameThis/";
}

FString UCMainBlueprintFL::MusicToPlaylistDir( const bool _Relative ) {
	return UHEVLibraryIO::UnpackDir( _Relative ) + "MusicToPlaylist/";
}
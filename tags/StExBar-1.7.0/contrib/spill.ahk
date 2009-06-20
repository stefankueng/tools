/*
USAGE:  
	<script> <temp_list> [flags]
	
	command for use in StExBar:
	
		with AutoHotKey installed (and using the uncompiled .ahk script):
		
			 <AutoHotKey.exe> <ahk_script> %selafile [flags]
			 
		using the compiled script:
		
			<compiled_script> %selafile [flags]
			
	(<AutoHotKey.exe> is "C:\Program Files\AutoHotKey\AutoHotKey.exe" on my computer...)

DESCRIPTION:
	Takes the contents of the folders listed in the given text file (first argument) and moves them into the parent directory.  Flags can be mixed and matched (and written in any order).

		move
			<folder_path>\<folder_name>\*.*
		to
			<folder_path>
		for all selected folders

FLAGS:
	r		-		Recusive.  Essentially takes every single item inside the selected folders' trees and puts them into their respective parent directories.
	
	d		-		Destructive.  All folders made empty are deleted.  Considering the above description, <folder_name> would be deleted only if this flag is present.  Combined with recusive mode, all folders involved are deleted (after being spilled out, of course).

COMMENTS:
	the code could probably be simplified to just a few lines...

CREATOR:
	Joshua A. Kinnison
	Submitted 2008-08-02 14:00
*/


destructive := false	
recursive := false

if 2 contains d
	destructive := true
if 2 contains r
	recursive := true



Loop Read, %1%
{
	item := A_LoopReadLine
	FileGetAttrib, Attributes, %item%
	IfInString Attributes, D
	{
		SplitPath item, name, dir, ext, name_no_ext, drive		

		if recursive
		{
			if not destructive
			{
				Loop %item%\*.*, 2, 1
				{	
					FileMove %A_LoopFileFullPath%\*.*, %dir%
					FileCreateDir %dir%\%A_LoopFileName%
				}
			}
			else
			{
				Loop %item%\*.*, 2, 1
				{	
					FileMove %A_LoopFileFullPath%\*.*, %dir%
				}
			}
		}
		else
		{
			FileMove %item%\*.*, %dir%
			Loop %item%\*.*, 2
			{
				FileMoveDir, %A_LoopFileFullPath%, %dir%\%A_LoopFileName%, 2
			}
		}

		FileRemoveDir %item%, 1
		
		if not destructive
			FileCreateDir %item%
	}
}
QUIT:
ExitApp

/*
USAGE:
	<script> <temp_list>
	
	command for use in StExBar:
	
		with AutoHotKey installed (and using the uncompiled .ahk script):
		
			 <AutoHotKey.exe> <ahk_script> %selafile
			 
		using the compiled script:
		
			<compiled_script> %selafile
			
	(<AutoHotKey.exe> is "C:\Program Files\AutoHotKey\AutoHotKey.exe" on my computer...)
	
DESCRIPTION:
	From the list contained in the <temp_list> file, any empty folders are deleted.
	
COMMENTS:
	really simple due to the way FileRemoveDir works (another flag is needed before it would remove a non-empty directory).
	
	Recursive searching could be used... but why bother when you can use
scripts like this on search results!
	
	By the way, selecting files doesn't matter -- they're just ignored, as you can't have a file named the same as a folder in the same directory (at least in Vista).
	
CREATOR:
	Joshua A. Kinnison
	Submitted 2008-08-02 14:00
*/

Loop Read, %1%
{
	folder := A_LoopReadLine
	FileRemoveDir %folder%
}

QUIT:
return

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
    Takes all the items listed in <temp_list> and puts them in a new folder.  Great for dealing with lots of files that should be in their own folders but are for some reason all mixed together in a single directory.

    A popup prompt will ask for the name of the folder, with the first file name provided as the default name.  Fancier suggestions could be made (perhaps by finding the common text among all considered items), but I never ended up coming back to try that out!

CREATOR:
    Joshua A. Kinnison
    Submitted 2008-08-02 14:00
*/


items =
name =

Loop Read, %1%  ;build a suggestion for the folder path.  better suggestions could be made, but I never got around to it.
{

    item := A_LoopReadLine
    SplitPath, item, name, dir, ext, name_no_ext, drive
    break
}
;msgbox, %item%--%name%--%dir%--%ext%--%name_no_ext%--%drive%
InputBox,fname,Folder Name?, Please enter the folder name.  The path will be relative to "%dir%" unless a full path is given,,,,,,,,%name_no_ext%       ;first item's name as the suggestion

if ErrorLevel
    return

;no need for trailing slashes
StringRight test, fname, 1
if test = \
    StringTrimRight, fname, fname, 1

;check if user provided path is relative or not
IfInString fname, :
    destination = %fname%
else
    destination = %dir%\%fname%

FileCreateDir %destination%

if ErrorLevel
{
    Msgbox there was some error in creating the folder "%dir%\%fname%"
    return
}

Loop Read, %1%
{
    item := A_LoopReadLine
    FileGetAttrib, Attributes, %item%
    IfInString, Attributes, D
    {
        StringGetPos, break,item,\,R1
        break:=break+1
        StringTrimLeft, tail, item, %break%
        FileCreateDir %destination%\%tail%
        FileMoveDir %item%, %destination%\%tail%, 2
    }
    else
        FileMove %item%, %destination%

    if ErrorLevel
        MsgBox error moving %item%                  ;meh
}
QUIT:
ExitApp

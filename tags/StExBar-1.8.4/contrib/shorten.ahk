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
    I made this script to cleanup folders after unzipping many things at once.  I always hated how some would zip their folders while others would zip their items.  I just ended up extracting to a folder every time and then checking each folder to see if it was redundant or not.  Like

        finding
            ...\mystuff\mystuff\<files>
        and fixing it to be
            ...\mystuff\<files>

    All this does is spills any folder listed in <temp_list> that contains precisely one folder and no files.

CREATOR:
    Joshua A. Kinnison
    Submitted 2008-08-02 14:00
*/

Loop Read, %1%
{
    folder := A_LoopReadLine

    files := 0
    loop, %folder%\*.*
        files++

    folders := 0
    loop, %folder%\*.*, 2
    {
        folders++
        real_folder = %A_LoopFileFullPath%
        real_fname = %A_LoopFileName%
    }

    if (files = 0) and (folders = 1)
    {
        StringGetPos,break,folder,\,R1
        StringLeft,dest,folder,%break%
        StringGetPos,break,dest,\,R1
        StringRight,dest_fname,dest,%break%
        ;MsgBox, changing %real_folder% to %dest%\%real_fname%
        FileMoveDir, %real_folder%, %dest%\%real_fname%, 2
        if ErrorLevel
            MsgBox, error changing %real_folder% to %dest%\%real_fname%
        else if  dest_fname <> %real_fname%
            FileRemoveDir, %folder%
    }
    if (files = 0) and (folders = 0)
        FileRemoveDir, %folder%
}

QUIT:
return

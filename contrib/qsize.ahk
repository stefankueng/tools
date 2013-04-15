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
    Calculates the size of the files and folders listed in the <temp_list> file (recursively, unlike the size listed in explorer's status bar).  The results are displayed in a tooltip (stays up for 5 seconds) that provides the total size and number of files examined.

    This script can be run before the previous run has finished.  All the results will pop up as they are ready, each with their own tooltip.  You could theoretically cover your screen with these tooltips!

    Alt-Enter already opens up the property page for selected items in explorer windows, but even that can be too much of a hassle when your working.  I found that using this script was much faster in situations where it had to be applied repeatedly (like trying to find out what combination of items will fit perfectly on a DVD).

    I found this thing invaluable when trying to do the above mentioned task on a slower network folder (right clicks took way too long, and I didn't need to see a full property page again and again...).

CREATOR:
    Joshua A. Kinnison
    Submitted 2008-08-02 14:00
*/
    unit_list =B KB MB GB TB PB EB ZB YB
    ;I await the day when I'll be using such large units!! (Oh, its only 1.4 ZB...  I'll bring it with me tomorrow)

    StringSplit, units, unit_list, %A_Space%
    TotalSize:=0

    num_files := 0

    Loop Read, %1%
    {
        file := A_LoopReadLine
        FileGetSize, size, %file%
        TotalSize += %size%

        FileGetAttrib, Attributes, %file%

        IfInString Attributes, D
        {
            Loop, %file%\*.*, , 1
            {
                num_files++
                TotalSize += %A_LoopFileSize%
            }
        }
        else
            num_files++
    }

    Loop
    {
        unit := units%A_Index%
        if TotalSize < 1024
            break
        TotalSize := TotalSize / 1024.00
    }

    ToolTip, Size: %TotalSize% %unit% in %num_files% files
    sleep 5000
    ToolTip
return

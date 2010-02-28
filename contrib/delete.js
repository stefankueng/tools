// delete.js
// this script deletes the selected paths to the recycle bin
//
// to use this in the StExBar, add the command line:
// wscript.exe "path/to/delete.js" //E:javascript %sel*paths
//

var objArgs,num;

objArgs = WScript.Arguments;
num = objArgs.length;
if (num < 1)
{
    WScript.Echo("Usage: [CScript | WScript] delete.js path/to/file");
    WScript.Quit(1);
}

var objShell = new ActiveXObject("Shell.Application")
var fs = new ActiveXObject("Scripting.FileSystemObject");
// remove the quotes
var filesstring = objArgs(0).replace(/\"(.*)\"/, "$1");
var files = filesstring.split("*");
var fileindex=0;
var errormsg = "";
while (fileindex < files.length)
{
	var f = files[fileindex];
	var item = objShell.Namespace(0).ParseName(f);
	item.InvokeVerb("delete");
    fileindex+=1;
}

WScript.Quit(0);

// attrib.js
// this script toggles the readonly file attribute of the file
// specified in the path.
//
// to use this in the StExBar, add the command line:
// wscript.exe "path/to/attrib.js" //E:javascript %selpaths
//

var objArgs,num;

objArgs = WScript.Arguments;
num = objArgs.length;
if (num != 1)
{
    WScript.Echo("Usage: [CScript | WScript] attrib.js path/to/file");
    WScript.Quit(1);
}

var fs, fso;
fs = new ActiveXObject("Scripting.FileSystemObject");
// remove the quotes
var filesstring = objArgs(0).replace(/\"(.*)\"/, "$1");
var files = filesstring.split("*");
var fileindex=0;
var errormsg = "";
while (fileindex < files.length)
{
	var f = files[fileindex];
	fso = fs.GetFile(f);
	if (fso.attributes & 1)
	{
		fso.attributes = fso.attributes - 1;
	}
	else
	{
		fso.attributes = fso.attributes + 1;
	}	
    fileindex+=1;
}
WScript.Quit(0);

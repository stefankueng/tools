$ErrorActionPreference = 'Stop'

$packageName= 'sktimestamp'
$url        = 'https://sourceforge.net/projects/stefanstools/files/SKTimeStamp/SKTimeStamp-1.3.6.msi/download' 
$url64      = 'https://sourceforge.net/projects/stefanstools/files/SKTimeStamp/SKTimeStamp64-1.3.6.msi/download' 
$Checksum   = '6283797870708DCEFDB1DD1A852CC97D0DF7945AEBE1D91311CE5EA2E26BF165'
$Checksum64 = '0E4B07449875E8ADC15BFB801C0745D44201D340DA84E74E7F58C0B1F0707284'

$packageArgs = @{
  packageName   = $packageName
  softwareName  = 'SKTimeStamp*'
  fileType      = 'MSI'
  url           = $url
  url64bit      = $url64
  checksum      = $Checksum
  checksum64    = $Checksum64
  checksumType  = 'sha256'
  silentArgs    = "/qn /norestart /l*v `"$($env:TEMP)\$($packageName).$($env:chocolateyPackageVersion).MsiInstall.log`"" # ALLUSERS=1 DISABLEDESKTOPSHORTCUT=1 ADDDESKTOPICON=0 ADDSTARTMENU=0
  validExitCodes= @(0, 3010, 1641)
}

Install-ChocolateyPackage @packageArgs

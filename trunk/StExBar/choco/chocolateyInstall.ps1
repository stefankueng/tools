Install-ChocolateyPackage 'StExBar' 'msi' '/quiet' 'http://sourceforge.net/projects/stefanstools/files/StExBar/StExBar-$MajorVersion$.$MinorVersion$.$MicroVersion$.msi/download' 'http://sourceforge.net/projects/stefanstools/files/StExBar/StExBar64-$MajorVersion$.$MinorVersion$.$MicroVersion$.msi/download' -checksum '$checksum$' -checksum64 '$checksum64$' -checksumType 'sha256'
; Gobby Windows Installer
; TODO: Remove gobby configuration file from all users directory on uninstall

[Setup]
AppName=Gobby
AppVerName=Gobby 0.5.0
DefaultDirName={autopf}\Gobby
DefaultGroupName=Gobby
UninstallDisplayIcon={app}\Gobby-0.5.exe
Uninstallable=yes
AppPublisher=Armin Burgmeier
AppPublisherURL=https://gobby.github.io
AppVersion=0.5.0
OutputBaseFilename=gobby-0.5.0-x64
ArchitecturesInstallIn64BitMode=x64
Compression=lzma
SolidCompression=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

[Files]
Source: "bin/*"; DestDir: "{app}/bin"
Source: "etc/*"; DestDir: "{app}/etc"; Flags: recursesubdirs
Source: "lib/*"; DestDir: "{app}/lib"; Flags: recursesubdirs
Source: "share/*"; DestDir: "{app}/share"; Flags: recursesubdirs

[Icons]
Name: "{autoprograms}\{groupname}\Gobby"; Filename: "{app}\bin\Gobby-0.5.exe"
Name: "{autoprograms}\{groupname}\Uninstall Gobby"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Gobby"; Filename: "{app}\bin\Gobby-0.5.exe"

# CoreHoster
Hosting .NET Core dll in C++ Code
## Build under Windows: 
In Visual Studio Developer Command Prompt (64bit!) run ```build.bat```.

Open Solution "Standard.sln" in Visual Studio and build it. Copy ```Standard.dll``` to ```x64\Debug```.

Run ```x64\Debug\LoadNetCore```.

## Build under Linux:
Run ```build.sh```.

Build ```Standard.dll```. Copy it to ```bin/linux```

Run ```./bin/linux/LoadNetCode
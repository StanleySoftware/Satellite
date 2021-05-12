<h1 align="center">
  <br>
  <img src="SatelliteInstaller/SatelliteLogo.png" alt="Satellite logo by Stanley Software" width="100px">
  <br>
  Satellite
  <br>
</h1>

Satellite is a locator tool and library for managing multiple environments on the same PC. It aims to provide a 'call site agnostic' way of retrieving path details for the given checkout.
Main uses are:
  
* Querying the location of significant directories in your working copy. E.g. Determining the root of the working copy, a scripts folder, a binary folder, anything can be configured.
* Invoking executables in your working copy, without having to know their path. This could be useful if your working copy contains its own version of python, cmake, or any other binary tool.

<b> WARNING: Satellite is a development tool that invokes processes based on the contents of files, and is naturally vulnerable to exploitation. Only use Satellite on repositories that you know are trustworthy. </b>
  
## How does it work? ##

### Hooking into VCS ###
Satellite relies on the VCS being used for your working copy. It uses this to locate the root of your working copy as a reliable point to start resolving queries. Current supported VCS are:

* Git

### satellite.json ###
The first step is configuring your checkout with satellite.json files. The root of your working copy is required to have a satellite.json as a starting point for more complex queries. The json file simply acts as a map from a keyname to string:
```json
{
  "scripts" : "./Scripts",
  "python" : "./PythonFolder/satellite.json"
}
```
The entries can be used to hold values but can also be used to point to other satellite.json files.

### Through the command line ###
Satellite can be invoked through the command line to locate and call an executable in that working copy. Say we have a filesystem structure matching the following:

```
SomeDrive 
│
└───ProjectA
│   │   satellite.json
│   │
│   └───SomeFolder
│   |   │   hello.py
│   |   │   ...
│   |
│   └───PythonFolder
│       │   satellite.json
│       │   python.exe
|
└───ProjectB
│   │ ...
```

then, if our working directory is set to `SomeDrive:\ProjectA\SomeFolder` and we run the following on the command line:
```
satellite python:! --args "hello.py"
```
Satellite resolves `python` against the satellite.json file at the root of ProjectA, and then invokes the process at the path it has looked up (see section on queries for more details about this).

### By linking ###

Satellite is also provided as a library that can be linked into your own developments tools.

* SatelliteC.dll provides a C-style interface for using Satellite
* SatelliteManaged.dll provides a .NET core interface for using Satellite (via SatelliteC.dll)
* SatelliteLib.lib can be compiled as a static library and linked directly into other applications.

## Queries ##

Queries are composed of two types of symbol:

* 'keys' which are arbitrary strings used as lookups in the json files
* ':' seperators that represent a chaining of lookups between multiple files

As an example, the following query 'key1:key2:key3' is resolved the following way:

* Queries are always invoked at a target path. From the command line, the working directory is used if none is specified.
* From that target path, the satellite.json file in the root of the working copy is found: C:\MyProject\satellite.json
* key1 is looked up inside C:\MyProject\satellite.json and the corresponding value is '.\DirectoryA\satellite.json' which becomes 'C:\MyProject\DirectoryA\satellite.json'
* ':' interprets the last looked up value as a json file, and attempts to parse it
* key2 is looked up inside C:\MyProject\DirectoryA\satellite.json and the corresponding value is '.\DirectoryA\DirectoryB\satellite.json' which becomes 'C:\MyProject\DirectoryA\DirectoryB\satellite.json'
* ':' interprets the last looked up value as a json file, and attempts to parse it
* key3 is looked up inside C:\MyProject\DirectoryA\DirectoryB\satellite.json and the corresponding value is 'MediumRare'

Some things to note:
* If the value looked up preceding ':' is not the path to a .json file then the query is ill-formed.
* '!' is a special key indicating a runnable executable (and any args). When carrying out queries on the command line, '!' indicates that the looked up value should be run. e.g. 'key1:!' invokes a process using the value of the key '!' in the json file looked up by 'key1'.

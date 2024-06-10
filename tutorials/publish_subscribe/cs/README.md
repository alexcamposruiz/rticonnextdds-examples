# Tutorial: Publish-Subscribe

This code is part of the Connext Publish-Subscribe tutorial.

## Building the Example :wrench:

You can build the example following the instructions in the tutorial, or you can
build it as follows.

1. Setting up the environment for Connext:

Use the following script to configure your shell environment variables to run
Connext executables and load dynamic libraries.

* Linux

If you’re using Bash, run this:

```sh
source <installdir>/resource/scripts/rtisetenv_<architecture>.bash
```

If you’re using the Z shell, run this:

```sh
source <installdir>/resource/scripts/rtisetenv_<architecture>.zsh
```

* macOS

If you’re using Bash, run this:

```sh
source <installdir>/resource/scripts/rtisetenv_<architecture>.bash
```

If you’re using the Z shell, run this:

```sh
source <installdir>/resource/scripts/rtisetenv_<architecture>.zsh
```

If you’re using the tcsh shell, run this:

```sh
source <installdir>/resource/scripts/rtisetenv_<architecture>.tcsh
```

* Windows

```sh
<installdir>\resource\scripts\rtisetenv_<architecture>.bat
```

2. Generate the type files and the csproj file:

```sh
rtiddsgen -language C# -platform [net5|net6|net8] home_automation.idl
```

3. Build the applications:

```sh
dotnet build
```

## Running the Applications :rocket:

Running publisher
```sh
dotnet run -- pub
...
```

Running subscriber
```sh
dotnet run -- sub
...
```

Extra. Running subscriber with timestamp
```sh
dotnet run -- sub_timestamp
...
```


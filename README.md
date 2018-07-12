# yate
Yet *another* text editor.

## Building and Running
Yate is built with Bazel (https://bazel.build/) and is dependent on ncurses:
```
sudo apt-get update
sudo apt-get install ncurses-dev
```

Once those are installed, run
```
bazel build yate
```
to build, or
```
bazel run yate
```
to build and run.

The first run will take some time before the build is cached. It will require
internet connection to download the Bazel Protobuf packages.

## Configuring
The `.yate` configuration file follows the text_proto format (src/config.proto).
# CMakeProjectTemplate

## How to build

I'd advise creating a directory for your build.
then run the following commands

```
cd yourBuildDirectory
cmake .. -G Ninja -B . -D UPDATE_DEPS=ON -D BUILD_WERROR=OFF -D BUILD_TESTS=OFF -D CMAKE_BUILD_TYPE=Debug    
cmake .. -G Ninja -B . -D UPDATE_DEPS=OFF -D BUILD_WERROR=OFF -D BUILD_TESTS=OFF -D CMAKE_BUILD_TYPE=Debug    
```

this should generate those files
Officially I've only tested visual studio and ninja generation.
**do note that it IS important to run the two cmake commands.**

For some reason spirv tools consistently breaks my build UNLESS you first download the dependencies and then generate with UPDATE_DEPS=OFF

to build the program you can either run 
```
cmake --build .
```

or compile it with the visual studio generated files.
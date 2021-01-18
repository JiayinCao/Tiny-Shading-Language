This version llvm is exactly the official release of LLVM 10.0.0

Detail could be found from [here](https://github.com/llvm/llvm-project/releases/tag/llvmorg-10.0.0).
[Here](https://github.com/llvm/llvm-project/archive/llvmorg-10.0.0.zip) is the source code link from that page.

# Build LLVM on MacOS

## Bulid ARM Version

- Download the source code from that above link and extract it.
- Get to the root folder of llvm, which should be named 'llvm-project-llvmorg-10.0.0'.
- Mkdir a new folder, namely 'build' or whatever you prefer.
- Type in the following cmake command. MAKE SURE 'LLVM_TARGET_ARCH' is not 'host', which won't work at the time this page is written.
- Build llvm with 'make' command.
- Make install. This will install the files to 'CMAKE_INSTALL_PREFIX' folder defined before.


```
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="~" -DLLVM_TARGET_ARCH="AArch64" -DLLVM_TARGETS_TO_BUILD="AArch64" -DLLVM_BUILD_EXAMPLES=OFF ../llvm -DCMAKE_BUILD_TYPE=Release
```

## Build X86_64 Version

The process is very similar with the above steps, except there is a different CMake line
```
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="~" -DLLVM_TARGET_ARCH="host" -DLLVM_TARGETS_TO_BUILD="X86" -DLLVM_BUILD_EXAMPLES=OFF ../llvm -DCMAKE_BUILD_TYPE=Release
```
Please notice that instead of specifying a target arch, we can rely on 'host' to find the target of the buildig machine.

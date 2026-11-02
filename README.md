# MyRender

这个文件夹已包含 `third_party/glm`（GLM 1.0.3）和 `third_party/minifb`（下载的 MiniFB 源码）。无需再次从网络下载依赖。

## 编译与运行

在 PowerShell 中进入本文件夹后执行：

```powershell
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build
.\build\MyRender.exe
```

运行后会出现渐变色窗口。按 Esc 或关闭窗口退出。`src/main.cpp` 是程序入口，可以从这里继续写渲染代码。GLM 使用 `#include <glm/glm.hpp>`；MiniFB 使用 `#include <MiniFB.h>`。

如果使用 Visual Studio 的 CMake 工程，也可以直接打开本文件夹，选择 CMakeLists.txt 构建。MiniFB 在 Windows 上配置为 GDI 后端，因此不需要另装 OpenGL 开发包。

依赖的许可文件分别在 `third_party/glm/copying.txt` 和 `third_party/minifb/LICENSE`。

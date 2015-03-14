# 故紙堆 #

籌劃【小狼毫】時的筆記，總結了準備開發環境和一些第三方庫的步驟。
如今可能不靈了。

僅留作參考。

## How to Rime with the Code under Windows ##

用MSVC编译【小狼毫】，需要Visual Studio 2010或2008、cmake、svn等工具。
具体请按照代码库里的这份说明来做！
http://code.google.com/p/rimeime/source/browse/trunk/weasel/BuildInstructions.txt

只编译 librime，用 MinGW 也可以，以下是一些总结，但 **不再时时更新** 啦！

### 配置MinGW開發環境 ###

[由此下載工具包](http://code.google.com/p/rimeime/downloads/detail?name=mingw_workspace_setup_r166.zip)

拿到他，解壓縮後只需編輯其中的`setup.bat`改動幾項配置，再一執行，通過簡單的幾步即可聯網安裝所需的軟件和第三方庫、拉代碼，將一份完整的開發環境配置齊全。

以下说明，解释了脚本为你做了哪些事：

倘若自己動手來編譯，請參考以下資源：

【程序庫】

  * http://www.boost.org/

  * http://code.google.com/p/googletest/

【工具】

  * Subversion客戶端
    * [TortoiseSVN](http://tortoisesvn.net/downloads.html)
    * [或者用命令行工具](http://sourceforge.net/projects/win32svn/)

  * http://www.mingw.org/

  * http://www.cmake.org/
    * [好教材《CMake實踐》](http://sewm.pku.edu.cn/src/paradise/reference/CMake%20Practice.pdf)

### 如何以 MinGW 編譯 la rime ###

  * 安装 MinGW
    * 设安装到 `C:\MinGW`
    * 注意，要安裝上 mingw32-make 這個軟件包
    * 若 bin 目錄無有 make.exe，將 mingw32-make.exe 另存一份命名為 make.exe

  * 安装 CMake
    * 下载一份 `cmake-2.8.4-win32-x86.zip`
    * 解压缩到 `%ProgramFiles%\gnuwin32`

  * 下載一份Boost庫，解壓縮到一處備用，譬如 `D:\code\boost_1_43_0`

  * 請按下文另一篇指南下載編譯 Google Test 備用，【注意】要做「将 gtest 配置到 rime 开发环境」這個步驟

  * 安裝SVN客戶端，按此[說明](http://code.google.com/p/rimeime/source/checkout)取得源碼至一處工作目錄
    * 簽出的源碼路徑好比是 `D:\code\rimeime`；下文均以 `%RIME_ROOT%` 表示此位置

  * 造一個小小的批處理 `build.bat`
    * 請以文本編輯工程中的模板 `%RIME_ROOT%\build.bat`，修改TODO內容

  * 執行 `build.bat` ，由 CMake 生成 `Makefile` 、編譯、測試
    * 於 `%RIME_ROOT%\build` 中得到靜態庫 `lib\librime.a` 及測試程序 `bin\rime_test.exe`
    * 完成

### 怎样以 MinGW 编译 Google Test ###

  * 安装 MinGW
> > 设安装到 `C:\MinGW`

  * 安装 CMake
    * 下载一份 `cmake-2.8.4-win32-x86.zip`
    * 解压缩到 `%ProgramFiles%\gnuwin32`

  * 下载源码 `gtest-1.5.0.tar.bz2` ，解压缩到某工作目录，如 `D:\code`

  * 为了敲命令方便，我造了一个小小的批处理 `shell.bat` ，内容类似于这个样子：
```
    set PATH=C:\MinGW\bin;%ProgramFiles%\gnuwin32\bin;C:\Python27;%PATH%
    cd /d D:\code
    cmd
```

  * 为下文引述方便，定义：
```
    set GTEST_ROOT=D:\code\gtest-1.5.0
```

  * 读一读 `%GTEST_ROOT%\README` ，发现 MinGW 并不在支持的编译环境之列；Google 到墙外，终得一法（感谢！）：
    * 对准 `%GTEST_ROOT%\CMakefileList.txt` 打这个 [patch](http://rimeime.googlecode.com/svn/trunk/data/gtest_mingw_hack.patch) ：
```
      59c59,61
      < find_package(Threads)
      ---
      > #find_package(Threads)
      > set(CMAKE_USE_PTHREADS_INIT false)
      > set(CMAKE_THREAD_LIBS_INIT false)
```

  * 运用刚才写的 `shell.bat`
```
      cd /d "%GTEST_ROOT%"
      mkdir mybuild
      cd mybuild
      cmake -G "MinGW Makefiles" ..
```
    * 若一切正常，已产生了 `Makefile` ；继续编译
```
      make
```

  * 忽略许多警告，得到.a及.exe文件
    * 运行 `gtest_unittest.exe` 测试编译成果

  * 将 gtest 配置到 rime 开发环境
    * 设 `RIME_ROOT` 为 rimeime 的顶层代码目录：
    * 将 `libgtest.a` 和 `libgtest_main.a` 复制到 `%RIME_ROOT%\thirdparty\lib` 内备用
    * 将 `%GTEST_ROOT%\include\gtest` 目录复制为 `%RIME_ROOT%\thirdparty\include\gtest` 备用
    * 完成

### 怎样以 MinGW 编译 D-Bus ###

這篇指南，已经用不到了，因为【小狼毫】放弃了移植IBus的努力转而用Windows原生的机制来开发。留作参考：

  * 安装 MinGW
> > 设安装到 `C:\MinGW`

  * 安装 CMake
    * 下载一份 `cmake-2.8.4-win32-x86.zip`
    * 解压缩到 `%ProgramFiles%\gnuwin32`

  * 安装一份 libxml2
    * 这里我用预编译好的二进制 `libxml2-2.7.7.win32.zip`
    * 解压缩到 `%ProgramFiles%\gnuwin32` (GNU的工具都放一处好统一设置PATH)

  * 下载源码 `dbus-1.4.1.tar.gz` ，解压缩到某工作目录，如 `D:\code`

  * 为了敲命令方便，我造了一个小小的批处理 `shell.bat` ，内容类似于这个样子：
```
    set PATH=C:\MinGW\bin;%ProgramFiles%\gnuwin32\bin;C:\Python27;%PATH%
    cd /d D:\code
    cmd
```

  * 为下文引述方便，定义：
```
    set DBUS_ROOT=D:\code\dbus-1.4.1
```

  * 读一读 `%DBUS_ROOT%\cmake\readme-cmake.txt` ，你就会了。接下来我再重复一遍：

  * 运用刚才写的 `shell.bat`
```
      cd /d "%DBUS_ROOT%"
      mkdir mybuild
      cd mybuild
      cmake -G "MinGW Makefiles" ..\cmake
```
    * 若一切正常，已产生了 `Makefile` ；继续编译、安装到默认的位置 `%ProgramFiles%\dbus`
```
      make
      make install
```
    * 若要指定安装位置：如安装到 `%ProgramFiles%\gnuwin32` ，须在以上步骤中为 `cmake` 多传入一个参数：
```
      cmake -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX="%ProgramFiles%\gnuwin32" ..\cmake
```

  * 完成
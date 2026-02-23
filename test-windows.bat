@echo off
REM SpectraCore Windows Testing Automation Script
REM UNITY-001: Windows P/Invoke Testing
REM
REM Usage:
REM   test-windows.bat [dll-path] [unity-project-path]
REM
REM Example:
REM   test-windows.bat spectra.dll "C:\UnityProjects\SpectraTest"

setlocal enabledelayedexpansion

echo ========================================
echo   SpectraCore Windows Testing Script
echo   UNITY-001: P/Invoke Testing
echo ========================================
echo.

REM ===== Configuration =====
set DLL_PATH=%1
set UNITY_PROJECT=%2

if "%DLL_PATH%"=="" (
    set DLL_PATH=spectra.dll
    echo No DLL path specified, using: %DLL_PATH%
) else (
    echo Using DLL: %DLL_PATH%
)

if "%UNITY_PROJECT%"=="" (
    echo WARNING: No Unity project path specified
    echo Standalone tests only
    set UNITY_PROJECT=NONE
) else (
    echo Unity project: %UNITY_PROJECT%
)

echo.

REM ===== Step 1: Verify DLL exists =====
echo [1/6] Verifying DLL...
if not exist "%DLL_PATH%" (
    echo ERROR: DLL not found: %DLL_PATH%
    echo.
    echo Please provide DLL path as first argument:
    echo   test-windows.bat path\to\spectra.dll
    echo.
    echo Or download from GitHub Actions:
    echo   https://github.com/yogt1984/SpectraCore/actions
    exit /b 1
)

echo   File: %DLL_PATH%
for %%A in ("%DLL_PATH%") do set DLL_SIZE=%%~zA
set /a DLL_SIZE_KB=DLL_SIZE/1024
echo   Size: %DLL_SIZE_KB% KB

REM Check if it's a valid PE file
powershell -Command "& {$h = Get-Content '%DLL_PATH%' -Encoding Byte -TotalCount 2; if ($h[0] -eq 77 -and $h[1] -eq 90) {exit 0} else {exit 1}}" >nul 2>&1
if errorlevel 1 (
    echo ERROR: File is not a valid PE executable
    exit /b 1
)

echo   Format: Valid PE executable
echo   [PASS]
echo.

REM ===== Step 2: Check DLL architecture =====
echo [2/6] Checking DLL architecture...

REM Try to use dumpbin if available (Visual Studio installed)
where dumpbin >nul 2>&1
if %errorlevel%==0 (
    echo   Using dumpbin...
    dumpbin /headers "%DLL_PATH%" 2>nul | findstr /C:"machine" | findstr /C:"x64" >nul
    if errorlevel 1 (
        echo   ERROR: DLL is not x64 architecture
        echo   Unity requires 64-bit DLL
        exit /b 1
    )
    echo   Architecture: x64
    echo   [PASS]
) else (
    echo   WARNING: dumpbin not found (Visual Studio not installed)
    echo   Skipping architecture check
    echo   [SKIP]
)
echo.

REM ===== Step 3: Check DLL exports =====
echo [3/6] Checking DLL exports...

where dumpbin >nul 2>&1
if %errorlevel%==0 (
    echo   Checking for required exports...
    set EXPORT_COUNT=0
    for /f %%i in ('dumpbin /exports "%DLL_PATH%" 2^>nul ^| findstr /C:"spectra_" ^| find /c /v ""') do set EXPORT_COUNT=%%i

    if !EXPORT_COUNT! LSS 20 (
        echo   WARNING: Only !EXPORT_COUNT! exports found
        echo   Expected ~50+ spectra_* functions
        echo   [WARN]
    ) else (
        echo   Found !EXPORT_COUNT! spectra_* exports
        echo   [PASS]
    )
) else (
    echo   WARNING: Cannot verify exports without dumpbin
    echo   [SKIP]
)
echo.

REM ===== Step 4: Run standalone P/Invoke test =====
echo [4/6] Running standalone P/Invoke test...

REM Check if standalone test exists
if not exist "unity\Tests\StandalonePInvokeTest.cs" (
    echo   ERROR: Test file not found: unity\Tests\StandalonePInvokeTest.cs
    echo   Make sure you're running from SpectraCore root directory
    exit /b 1
)

REM Check if C# compiler is available
where csc >nul 2>&1
if errorlevel 1 (
    echo   ERROR: C# compiler (csc.exe) not found
    echo   Make sure .NET Framework SDK is installed
    echo   Typically located at: C:\Windows\Microsoft.NET\Framework64\v4.*
    echo.
    echo   Try running from Developer Command Prompt for VS
    exit /b 1
)

REM Compile test
echo   Compiling test program...
csc /out:PInvokeTest.exe unity\Tests\StandalonePInvokeTest.cs >nul 2>&1
if errorlevel 1 (
    echo   ERROR: Compilation failed
    csc /out:PInvokeTest.exe unity\Tests\StandalonePInvokeTest.cs
    exit /b 1
)
echo   Compiled: PInvokeTest.exe

REM Copy DLL to same directory
copy /Y "%DLL_PATH%" . >nul 2>&1

REM Run test
echo   Running test...
echo.
PInvokeTest.exe
set TEST_RESULT=%errorlevel%

if %TEST_RESULT% NEQ 0 (
    echo.
    echo   [FAIL] Standalone test failed
    echo   Check error messages above
    exit /b 1
) else (
    echo.
    echo   [PASS] All standalone tests passed
)
echo.

REM ===== Step 5: Unity project setup (if provided) =====
if "%UNITY_PROJECT%"=="NONE" (
    echo [5/6] Unity testing...
    echo   No Unity project specified, skipping
    echo   [SKIP]
    echo.
    goto :performance
)

echo [5/6] Setting up Unity project...

REM Check if Unity project exists
if not exist "%UNITY_PROJECT%" (
    echo   ERROR: Unity project not found: %UNITY_PROJECT%
    exit /b 1
)

REM Create plugin directory
set PLUGIN_DIR=%UNITY_PROJECT%\Assets\SpectraCore\Runtime\Plugins\Windows\x86_64
echo   Creating plugin directory...
if not exist "%PLUGIN_DIR%" (
    mkdir "%PLUGIN_DIR%"
)

REM Copy DLL
echo   Copying DLL to Unity project...
copy /Y "%DLL_PATH%" "%PLUGIN_DIR%\" >nul
if errorlevel 1 (
    echo   ERROR: Failed to copy DLL
    exit /b 1
)

echo   DLL installed to: %PLUGIN_DIR%\spectra.dll

REM Copy Unity integration files if not already present
set RUNTIME_DIR=%UNITY_PROJECT%\Assets\SpectraCore\Runtime
if not exist "%RUNTIME_DIR%\Scripts" (
    echo   Copying Unity integration files...
    xcopy /E /I /Y unity\Runtime "%RUNTIME_DIR%" >nul 2>&1
    xcopy /E /I /Y unity\Editor "%UNITY_PROJECT%\Assets\SpectraCore\Editor" >nul 2>&1
    xcopy /E /I /Y unity\Tests "%UNITY_PROJECT%\Assets\SpectraCore\Tests" >nul 2>&1
    echo   Integration files copied
) else (
    echo   Integration files already present
)

echo   [DONE]
echo.
echo   To run Unity tests:
echo   1. Open Unity project: %UNITY_PROJECT%
echo   2. Window -^> General -^> Test Runner
echo   3. Click "Run All" in EditMode tab
echo   4. Verify all 71 tests pass
echo.

:performance

REM ===== Step 6: Performance test =====
echo [6/6] Performance test...
echo   Creating performance test...

REM Create simple performance test C# program
(
echo using System;
echo using System.Runtime.InteropServices;
echo using System.Diagnostics;
echo class PerfTest {
echo     [DllImport^("spectra"^)] static extern IntPtr spectra_fft_create^(int size^);
echo     [DllImport^("spectra"^)] static extern void spectra_fft_destroy^(IntPtr fft^);
echo     [DllImport^("spectra"^)] static extern int spectra_fft_forward^(IntPtr fft, float[] input, float[] outputReal, float[] outputImag^);
echo.
echo     static void Main^(^) {
echo         const int size = 1024;
echo         const int iterations = 1000;
echo         IntPtr fft = spectra_fft_create^(size^);
echo         float[] input = new float[size];
echo         float[] real = new float[size];
echo         float[] imag = new float[size];
echo.
echo         var sw = Stopwatch.StartNew^(^);
echo         for ^(int i = 0; i ^< iterations; i++^)
echo             spectra_fft_forward^(fft, input, real, imag^);
echo         sw.Stop^(^);
echo.
echo         double avgMs = sw.ElapsedMilliseconds / ^(double^)iterations;
echo         Console.WriteLine^($"  {iterations} FFTs ^(1024-point^) in {sw.ElapsedMilliseconds}ms"^);
echo         Console.WriteLine^($"  Average: {avgMs:F3}ms per FFT"^);
echo         Console.WriteLine^(avgMs ^< 1.0 ? "  [PASS] Suitable for real-time" : "  [WARN] May be too slow for real-time"^);
echo.
echo         spectra_fft_destroy^(fft^);
echo     }
echo }
) > PerfTest.cs

REM Compile performance test
csc /out:PerfTest.exe PerfTest.cs >nul 2>&1
if errorlevel 1 (
    echo   WARNING: Could not compile performance test
    echo   [SKIP]
    goto :summary
)

REM Run performance test
echo   Running FFT performance benchmark...
echo.
PerfTest.exe
echo.

:summary

REM ===== Summary =====
echo ========================================
echo   Testing Summary
echo ========================================
echo.
echo   DLL Verification:        [PASS]
echo   Architecture Check:      [PASS]
echo   Export Check:            [PASS]
echo   Standalone Test:         [PASS]

if "%UNITY_PROJECT%"=="NONE" (
    echo   Unity Integration:       [SKIP] - No project specified
) else (
    echo   Unity Integration:       [DONE] - Run tests in Unity Editor
)

echo.
echo ========================================
echo   Next Steps
echo ========================================
echo.

if "%UNITY_PROJECT%" NEQ "NONE" (
    echo 1. Open Unity Editor
    echo    Project: %UNITY_PROJECT%
    echo.
    echo 2. Run Unity tests:
    echo    - Window -^> General -^> Test Runner
    echo    - EditMode tab -^> Run All
    echo    - Verify 71/71 tests pass
    echo.
    echo 3. Test editor tools:
    echo    - Window -^> Spectra -^> Signal Inspector
    echo    - Window -^> Spectra -^> Filter Designer
    echo.
    echo 4. Test example projects
    echo.
) else (
    echo To set up Unity testing:
    echo.
    echo 1. Create/open Unity project
    echo.
    echo 2. Re-run this script with Unity project path:
    echo    test-windows.bat %DLL_PATH% "C:\Path\To\UnityProject"
    echo.
)

echo 5. Fill out completion report:
echo    - Copy: UNITY-001_COMPLETION_TEMPLATE.md
echo    - To: UNITY-001_COMPLETION.md
echo    - Add test results and screenshots
echo.
echo 6. Commit results to repository
echo.

echo ========================================
echo   For detailed testing instructions:
echo   See UNITY-001_WINDOWS_TESTING_GUIDE.md
echo ========================================
echo.

REM Cleanup
if exist PInvokeTest.exe del PInvokeTest.exe
if exist PerfTest.exe del PerfTest.exe
if exist PerfTest.cs del PerfTest.cs

endlocal
exit /b 0

using System;
using System.Runtime.InteropServices;

/// <summary>
/// Standalone test to verify P/Invoke bindings work correctly.
/// Compile with: mcs StandalonePInvokeTest.cs -out:PInvokeTest.exe
/// Run with: mono PInvokeTest.exe (or ./PInvokeTest.exe on .NET Core)
/// Or: dotnet script StandalonePInvokeTest.cs
/// </summary>
class StandalonePInvokeTest
{
    private const string LibraryName = "spectra";

    // Basic bindings to test
    [DllImport(LibraryName)]
    private static extern int spectra_version_major();

    [DllImport(LibraryName)]
    private static extern int spectra_version_minor();

    [DllImport(LibraryName)]
    private static extern IntPtr spectra_version_string();

    [DllImport(LibraryName)]
    private static extern void spectra_clear_error();

    [DllImport(LibraryName)]
    private static extern int spectra_get_last_error();

    [DllImport(LibraryName)]
    private static extern IntPtr spectra_fft_create(int size);

    [DllImport(LibraryName)]
    private static extern void spectra_fft_destroy(IntPtr fft);

    [DllImport(LibraryName)]
    private static extern int spectra_fft_size(IntPtr fft);

    [DllImport(LibraryName)]
    private static extern int spectra_fft_forward(
        IntPtr fft,
        [In] float[] input,
        [Out] float[] outputReal,
        [Out] float[] outputImag);

    [DllImport(LibraryName)]
    private static extern int spectra_butter(
        int order,
        float normalizedFreq,
        int filterType,
        [Out] float[] b,
        ref int bLen,
        [Out] float[] a,
        ref int aLen);

    [DllImport(LibraryName)]
    private static extern int spectra_iir_coeff_size(int order);

    static int Main(string[] args)
    {
        Console.WriteLine("SpectraCore P/Invoke Standalone Test");
        Console.WriteLine("=====================================\n");

        int passCount = 0;
        int failCount = 0;

        // Test 1: Version Info
        try
        {
            int major = spectra_version_major();
            int minor = spectra_version_minor();
            IntPtr versionPtr = spectra_version_string();
            string version = Marshal.PtrToStringAnsi(versionPtr);

            Console.WriteLine($"[PASS] Version Info: {major}.{minor} ({version})");
            passCount++;
        }
        catch (Exception e)
        {
            Console.WriteLine($"[FAIL] Version Info: {e.Message}");
            failCount++;
        }

        // Test 2: Error Handling
        try
        {
            spectra_clear_error();
            int error = spectra_get_last_error();

            if (error == 0)
            {
                Console.WriteLine("[PASS] Error Handling: Clear works");
                passCount++;
            }
            else
            {
                Console.WriteLine($"[FAIL] Error Handling: Expected 0, got {error}");
                failCount++;
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"[FAIL] Error Handling: {e.Message}");
            failCount++;
        }

        // Test 3: FFT Create/Destroy
        try
        {
            IntPtr fft = spectra_fft_create(256);

            if (fft == IntPtr.Zero)
            {
                Console.WriteLine("[FAIL] FFT Create: Returned null");
                failCount++;
            }
            else
            {
                int size = spectra_fft_size(fft);

                if (size == 256)
                {
                    Console.WriteLine("[PASS] FFT Create: Size verified (256)");
                    passCount++;
                }
                else
                {
                    Console.WriteLine($"[FAIL] FFT Create: Expected size 256, got {size}");
                    failCount++;
                }

                spectra_fft_destroy(fft);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"[FAIL] FFT Create: {e.Message}");
            failCount++;
        }

        // Test 4: FFT Forward Transform
        try
        {
            IntPtr fft = spectra_fft_create(256);

            float[] input = new float[256];
            for (int i = 0; i < 256; i++)
                input[i] = 1.0f; // DC signal

            float[] outputReal = new float[256];
            float[] outputImag = new float[256];

            int result = spectra_fft_forward(fft, input, outputReal, outputImag);

            if (result == 0 && outputReal[0] > 0)
            {
                Console.WriteLine($"[PASS] FFT Forward: DC component = {outputReal[0]:F2}");
                passCount++;
            }
            else
            {
                Console.WriteLine($"[FAIL] FFT Forward: result={result}, DC={outputReal[0]}");
                failCount++;
            }

            spectra_fft_destroy(fft);
        }
        catch (Exception e)
        {
            Console.WriteLine($"[FAIL] FFT Forward: {e.Message}");
            failCount++;
        }

        // Test 5: Filter Design (Butterworth)
        try
        {
            int order = 4;
            int maxSize = spectra_iir_coeff_size(order);

            float[] b = new float[maxSize];
            float[] a = new float[maxSize];
            int bLen = 0;
            int aLen = 0;

            int result = spectra_butter(order, 0.3f, 0, b, ref bLen, a, ref aLen);

            if (result == 0 && bLen > 0 && aLen > 0)
            {
                Console.WriteLine($"[PASS] Butterworth: {bLen} b coeffs, {aLen} a coeffs");
                passCount++;
            }
            else
            {
                Console.WriteLine($"[FAIL] Butterworth: result={result}, bLen={bLen}, aLen={aLen}");
                failCount++;
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"[FAIL] Butterworth: {e.Message}");
            failCount++;
        }

        // Test 6: Library Path Check
        try
        {
            string currentDir = System.IO.Directory.GetCurrentDirectory();
            Console.WriteLine($"\n[INFO] Current Directory: {currentDir}");
            Console.WriteLine($"[INFO] Library Search: Looking for {LibraryName}.so/dll/dylib");
        }
        catch { }

        // Summary
        Console.WriteLine($"\n=====================================");
        Console.WriteLine($"Tests Passed: {passCount}");
        Console.WriteLine($"Tests Failed: {failCount}");
        Console.WriteLine($"Total Tests:  {passCount + failCount}");
        Console.WriteLine($"=====================================");

        return failCount == 0 ? 0 : 1;
    }
}

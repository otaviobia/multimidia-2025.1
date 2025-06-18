@echo off
REM Script to test the compressor/decompressor for all quality levels (1-100) on Windows.
REM
REM Usage: test_all_qualities.bat <input_image.bmp>
REM Example: test_all_qualities.bat images\lenna.bmp

SETLOCAL EnableDelayedExpansion

REM 1. Check if an input image was provided as an argument
if "%~1"=="" (
    echo Usage: %0 ^<input_image.bmp^>
    exit /b 1
)

SET "INPUT_IMAGE=%~1"

REM 2. Check if the input image exists
if not exist "%INPUT_IMAGE%" (
    echo Error: Input file '%INPUT_IMAGE%' not found.
    exit /b 1
)

REM 3. Check if the executables exist (assuming .exe extension)
if not exist "compressor.exe" (
    echo Error: Make sure 'compressor.exe' is compiled and is in the current directory.
    exit /b 1
)
if not exist "decompressor.exe" (
    echo Error: Make sure 'decompressor.exe' is compiled and is in the current directory.
    exit /b 1
)

REM 4. Create directories for the output files (the >nul 2>nul suppresses any errors if they already exist)
echo Creating output directories...
mkdir compressed_files >nul 2>nul
mkdir reconstructed_images >nul 2>nul

REM 5. Main loop that iterates from 1 to 100
FOR /L %%q IN (1, 1, 100) DO (
    echo --- Testing Quality: %%q/100 ---

    REM Define unique filenames for this quality level
    SET "COMPRESSED_FILE=compressed_files\compressed_q%%q.bin"
    SET "RECONSTRUCTED_FILE=reconstructed_images\reconstructed_q%%q.bmp"

    REM Run the compressor
    echo Compressing with quality %%q...
    compressor.exe "%INPUT_IMAGE%" "!COMPRESSED_FILE!" %%q

    REM Check if compression was successful before decompressing
    if exist "!COMPRESSED_FILE!" (
        REM Run the decompressor
        echo Decompressing to "!RECONSTRUCTED_FILE!"...
        decompressor.exe "!COMPRESSED_FILE!" "!RECONSTRUCTED_FILE!"
    ) else (
        echo Error: Compressed file '!COMPRESSED_FILE!' was not created. Skipping decompression.
    )
)

echo.
echo --- All tests were completed! ---
echo Compressed files are in the 'compressed_files' directory.
echo Reconstructed images are in the 'reconstructed_images' directory.

ENDLOCAL
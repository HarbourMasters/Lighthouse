Using Namespace System

$url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.6/LLVM-14.0.6-win64.exe"
$llvmInstallerPath = ".\LLVM-14.0.6-win64.exe"
$clangFormatFilePath = ".\clang-format.exe"
$requiredVersion = "clang-format version 14.0.6"
$currentVersion = ""

# --- Dynamically find 7-Zip anywhere in PATH ---
function Get-7ZipPath {
    $paths = $env:Path -split ';'
    foreach ($p in $paths) {
        $exe = Join-Path $p "7z.exe"
        if (Test-Path $exe -PathType Leaf) {
            return $exe
        }
    }
	$defPath = "C:\\Program Files\\7-Zip\\7z.exe"
	if (Test-Path $defPath -PathType Leaf) {
		return $defPath;
	}
    return $null
}

# --- Check existing clang-format ---
if (Test-Path $clangFormatFilePath) {
    $currentVersion = & $clangFormatFilePath --version
    if ($currentVersion -ne $requiredVersion) {
        Remove-Item $clangFormatFilePath -Force
    }
}

$sevenZipPath = Get-7ZipPath
if (-not $clangFormatFilePath -and -not $sevenZipPath) {
    Write-Host "7-Zip is not installed or not on PATH. Please install 7-Zip and try again."
    exit
}

# --- Download and extract clang-format if needed ---
if (-not (Test-Path $clangFormatFilePath)) {
    $wc = New-Object Net.WebClient
    $wc.DownloadFile($url, $PSScriptRoot + $llvmInstallerPath)

    $specificFileInArchive = "bin\clang-format.exe"
    & "$sevenZipPath" e $llvmInstallerPath $specificFileInArchive -aoa

    Remove-Item $llvmInstallerPath -Force
}

# --- Find and format source files ---
$basePath = (Resolve-Path .).Path
$files = Get-ChildItem -Path "$basePath\src\port" -Recurse -File |
    Where-Object { 
        ($_.Extension -in '.c', '.cpp', '.h', '.hpp') -and
        (-not ($_.FullName -like "*\build*" -or $_.FullName -like "*\core1\*"))
    }

for ($i = 0; $i -lt $files.Length; $i++) {
    $file = $files[$i]
    $relativePath = $file.FullName.Substring($basePath.Length + 1)
    Write-Host "Formatting [$($i+1)/$($files.Length)] $relativePath"
    .\clang-format.exe -i $file.FullName
}
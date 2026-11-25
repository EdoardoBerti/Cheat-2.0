<#
.SYNOPSIS
    Analizzatore automatico per dump Il2CppDumper (dump.cs / script.json)
    Estrae gli offset delle classi di gioco per ROB RUN e li formatta per offsets.h.

.PARAMETER DumpFile
    Percorso al file dump.cs generato da Il2CppDumper.

.EXAMPLE
    .\parse_il2cpp_dump.ps1 -DumpFile "C:\Il2CppDumper\dump.cs"
#>

param(
    [Parameter(Mandatory = $false)]
    [string]$DumpFile = "dump.cs"
)

if (-not (Test-Path $DumpFile)) {
    Write-Host "[!] File '$DumpFile' non trovato!" -ForegroundColor Red
    Write-Host "[*] Utilizzo: .\parse_il2cpp_dump.ps1 -DumpFile percorso\a\dump.cs" -ForegroundColor Yellow
    exit 1
}

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "   RONRUN - Il2CppDumper Offset Finder" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "[*] Analisi di: $DumpFile" -ForegroundColor Green

$targets = @(
    "GameManager",
    "PlayerController",
    "PlayerCharacter",
    "Character",
    "NetworkPlayer",
    "Player",
    "Fence",
    "Vendor",
    "ShopManager",
    "Inventory",
    "ItemInstance",
    "WeaponController",
    "Weapon"
)

Write-Host "[*] Ricerca delle classi target in corso..." -ForegroundColor Gray

$lines = Get-Content $DumpFile
$currentClass = $null
$capture = $false
$foundClasses = @{}

for ($i = 0; $i -lt $lines.Count; $i++) {
    $line = $lines[$i].Trim()

    if ($line -match "^// Namespace:\s*(.*)") {
        $namespace = $matches[1]
    }

    if ($line -match "^public class\s+([A-Za-z0-9_]+)") {
        $className = $matches[1]
        foreach ($target in $targets) {
            if ($className -like "*$target*") {
                $currentClass = $className
                $foundClasses[$currentClass] = @()
                $capture = $true
                break
            }
        }
    }
    elseif ($capture -and $line -match "^}") {
        $capture = $false
        $currentClass = $null
    }
    elseif ($capture -and $line -match "// Offset:\s*(0x[0-9A-Fa-f]+)") {
        $offset = $matches[1]
        $foundClasses[$currentClass] += "$line (Offset: $offset)"
    }
}

Write-Host "`n[+] Classi individuate nel dump:" -ForegroundColor Green
foreach ($key in $foundClasses.Keys) {
    Write-Host "`n================ Class: $key ================" -ForegroundColor Yellow
    $fields = $foundClasses[$key] | Select-Object -First 25
    foreach ($f in $fields) {
        Write-Host "  $f"
    }
    if ($foundClasses[$key].Count -gt 25) {
        Write-Host "  ... (+ $($foundClasses[$key].Count - 25) altri campi)" -ForegroundColor DarkGray
    }
}

Write-Host "`n[+] Analisi completata. Copia gli offset desiderati in cheat/offsets.h" -ForegroundColor Cyan

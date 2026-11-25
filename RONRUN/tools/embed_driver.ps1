<#
.SYNOPSIS
    Converte il driver compilato ronrun_driver.sys in un array di byte C++
    e lo incorpora in cheat/driver_payload.h per produrre un eseguibile 100% monolitico.

.PARAMETER DriverPath
    Percorso al file .sys (default: driver/ronrun_driver.sys o ronrun_driver.sys)
#>

param(
    [string]$DriverPath = "ronrun_driver.sys"
)

if (-not (Test-Path $DriverPath)) {
    if (Test-Path "driver\ronrun_driver.sys") {
        $DriverPath = "driver\ronrun_driver.sys"
    } else {
        Write-Host "[!] File driver '$DriverPath' non trovato." -ForegroundColor Yellow
        Write-Host "[*] Verrà mantenuto il payload vuoto (modalità diretta usermode abilitata)." -ForegroundColor Cyan
        exit 0
    }
}

Write-Host "[*] Lettura del driver: $DriverPath..." -ForegroundColor Green
$bytes = [System.IO.File]::ReadAllBytes((Resolve-Path $DriverPath))
$size = $bytes.Length

$sb = [System.Text.StringBuilder]::new()
[void]$sb.AppendLine("#pragma once")
[void]$sb.AppendLine("// Auto-generated from $DriverPath ($size bytes)")
[void]$sb.AppendLine("#include <cstdint>")
[void]$sb.AppendLine("#include <cstddef>")
[void]$sb.AppendLine("")
[void]$sb.AppendLine("inline const uint8_t g_driver_payload[] = {")

for ($i = 0; $i -lt $bytes.Length; $i++) {
    if ($i % 16 -eq 0) {
        if ($i -gt 0) { [void]$sb.AppendLine(",") }
        [void]$sb.Append("    ")
    } else {
        [void]$sb.Append(", ")
    }
    [void]$sb.AppendFormat("0x{0:X2}", $bytes[$i])
}

[void]$sb.AppendLine("")
[void]$sb.AppendLine("};")
[void]$sb.AppendLine("inline const size_t g_driver_payload_size = $size;")

$dest = "cheat\driver_payload.h"
[System.IO.File]::WriteAllText($dest, $sb.ToString())
Write-Host "[+] Driver incorporato con successo in $dest ($size bytes)!" -ForegroundColor Green

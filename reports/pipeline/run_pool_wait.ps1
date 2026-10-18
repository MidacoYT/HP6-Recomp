$proc = Start-Process -FilePath ".\build\Debug\hp6_pool_wait_smoke.exe" -RedirectStandardOutput "$env:TEMP\hps.out" -RedirectStandardError "$env:TEMP\hps.err" -NoNewWindow -PassThru
try {
    if (-not $proc.WaitForExit(20000)) {
        Stop-Process -Id $proc.Id -Force
        "TIMED OUT (killed)"
    } else {
        "exit=" + $proc.ExitCode
    }
} finally {
    Get-Content "$env:TEMP\hps.out" | Select-Object -Last 14
    "--- stderr ---"
    Get-Content "$env:TEMP\hps.err" | Select-Object -Last 6
}

# RONRUN — External Cheat Suite for Rob Run (Single Standalone Executable)

Architettura unificata in un **unico eseguibile autonomo** (`RONRUN.exe`) per test e analisi su **Rob Run** (Unity IL2CPP).

---

## Caratteristiche Principali

- **Architettura a Eseguibile Unico**: Non richiede driver separati o mapper esterni per funzionare.
- **Motore di Memoria Dual-Engine (Ibrido)**:
  - **Modalità Diretta Usermode (Standalone)**: Legge e scrive direttamente sulla memoria del processo di gioco (`OpenProcess`, `ReadProcessMemory`, `WriteProcessMemory`, `VirtualProtectEx`). Zero dipendenze esterne.
  - **Modalità Kernel Driver (Stealth IOCTL)**: Se il driver kernel è presente o caricato (es. tramite KDMapper o servizio), l'eseguibile lo rileva e si aggancia automaticamente per il bypass completo dell'anti-cheat a livello kernel.
  - **Payload Integrato Opzionale**: Possibilità di incorporare i byte del driver `.sys` direttamente dentro l'eseguibile (`tools/embed_driver.ps1`).
- **Overlay DirectX 11 Trasparente**: Finestra invisibile e click-through con render ImGui personalizzato.
- **Feature Set Completo**:
  - **Player ESP**: Box 2D, barre salute, nomi, distanze, snaplines, skeleton e filtro team.
  - **Loot ESP & Vacuum**: Evidenziazione oggetti a terra e teletrasporto istantaneo degli oggetti ai piedi del giocatore (`[E]`).
  - **Fast Search**: Apertura istantanea di casse e container senza ruota di caricamento.
  - **Aimbot**: Aggancio testa/collo/busto, smoothing configurabile e cerchio FOV dinamico.
  - **Silent Aim**: Aggancio invisibile tramite `AimTargetModel`.
  - **Movement**: Speedhack moltiplicatore e Noclip tridimensionale.
  - **Fence Money Exploit**: Override del valore di vendita a 10.000.000 (`[F8]`) per accumulo rapido di crediti.

---

## Struttura del Progetto

```
RONRUN/
├── cheat/                  # Usermode Overlay, GUI & Unified Memory Engine
│   ├── driver_comm.h       # Motore di memoria unificato (Kernel IOCTL + Usermode diretto)
│   ├── driver_payload.h    # Payload driver incorporato (opzionale per build monolitica)
│   ├── offsets.h           # Offset Unity / IL2CPP da dump Il2CppDumper
│   ├── sdk.h               # Strutture dati di gioco (Player, Camera, Loot, Container, Fence)
│   ├── math.h              # Vettori, matrici e WorldToScreen
│   ├── features.h          # ESP, Aimbot, Silent Aim, Speedhack, Noclip, Fence Exploit
│   ├── overlay.h           # Finestra trasparente DirectX 11 & ImGui loop
│   ├── menu.h              # Menu GUI interattivo ImGui con tab e stato backend
│   └── main.cpp            # Entry point orchestratore
├── driver/                 # Sorgente Driver Kernel opzionale
│   ├── driver.c            # Implementazione driver (MmCopyVirtualMemory, PID/Module lookup)
│   ├── driver.vcxproj      # Progetto Visual Studio WDK
│   └── ioctl.h             # Definizioni IOCTL condivise
├── tools/
│   ├── parse_il2cpp_dump.ps1 # Script per estrarre automaticamente offset da dump.cs
│   └── embed_driver.ps1      # Script per incorporare ronrun_driver.sys nell'eseguibile unico
├── vendor/imgui/           # Dear ImGui submodule
├── CMakeLists.txt          # Configurazione di compilazione per l'eseguibile unico
├── build.bat               # Compilazione one-click dell'eseguibile unico (RONRUN.exe)
├── setup.bat               # Script preparatorio
└── README.md
```

---

## Come Compilare l'Eseguibile Unico

Requisiti: **Visual Studio 2022** con carico di lavoro *"Sviluppo di applicazioni desktop con C++"* (include MSVC e CMake).

1. Esegui il file:
   ```cmd
   build.bat
   ```
2. Lo script:
   - Verifica e scarica automaticamente la libreria grafica ImGui.
   - Configura il progetto CMake x64.
   - Compila la configurazione Release.
   - Copia il binario finale nella root del progetto: **`RONRUN.exe`**.

---

## Come Usare RONRUN

1. Avvia **Rob Run** da Steam ed entra nel gioco o nel menu principale.
2. Esegui **`RONRUN.exe`** (preferibilmente come Amministratore).
3. La console mostrerà il backend attivo:
   - `[BACKEND: USERMODE DIRECT]` se non è presente alcun driver.
   - `[BACKEND: KERNEL DRIVER]` se è attivo il driver kernel per stealth bypass.
4. Premi i tasti di scelta rapida:
   - **`[INSERT]`**: Apri / Chiudi il menu grafico a schermo
   - **`[END]`**: Chiudi e termina il cheat in modo pulito
   - **`[RMB]` (Tasto destro del mouse)**: Tieni premuto per attivare l'Aimbot
   - **`[SHIFT]`**: Tieni premuto per lo Speedhack
   - **`[Mouse 4]` / `[XBUTTON1]`**: Noclip tridimensionale (W/A/S/D + Spazio/Ctrl)
   - **`[E]`**: Fast Loot (attira a terra tutti gli oggetti nel raggio selezionato)
   - **`[F8]`**: Attiva il Fence Exploit (imposta il valore degli oggetti nell'inventario a 10M)
   - **`[F9]`**: Ripristina i valori di vendita standard

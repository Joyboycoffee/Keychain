#!/usr/bin/env python3
"""
SPEARHEAD // Tactical ESP32-C3 Firmware Flasher Studio
Auto-scanning port detector & one-click firmware installer for Digi Keychain.
Supports both modern Desktop GUI and headless / terminal CLI automation.
"""

import sys
import os
import time
import re
import argparse
import subprocess
import threading
from pathlib import Path

if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

try:
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] pyserial is required. Install via: pip install pyserial")
    sys.exit(1)

# Base Paths
SCRIPT_DIR = Path(__file__).resolve().parent
BINS_DIR = SCRIPT_DIR / "assets" / "bins"

FACTORY_BIN = BINS_DIR / "spearhead_merged_factory_4mb.bin"
FIRMWARE_BIN = BINS_DIR / "firmware.bin"
LITTLEFS_BIN = BINS_DIR / "littlefs.bin"
ICON_PATH = SCRIPT_DIR / "favicon.ico"

# ESP Vendor & Product IDs
KNOWN_ESP_VIDS = {
    0x303A: "Espressif Native USB-Serial/JTAG (ESP32-C3)",
    0x1A86: "WCH USB-to-UART (CH340/CH9102)",
    0x10C4: "Silicon Labs CP210x (ESP USB-UART)",
    0x0403: "FTDI USB-UART (ESP Bridge)",
    0x067B: "Prolific USB-Serial",
}

# Ignore virtual or internal ports
IGNORED_PATTERNS = ["bthenum", "bluetooth", "acpi", "root", "com1\b", "standard serial over bluetooth"]


# =========================================================================
# PORT SCANNING & ESP AUTO-DETECTION ENGINE
# =========================================================================
def scan_ports():
    """Scans all serial ports, filters out non-ESP ports, and scores ESP candidates."""
    all_ports = serial.tools.list_ports.comports()
    candidates = []

    for port in all_ports:
        desc = (port.description or "").lower()
        hwid = (port.hwid or "").lower()
        device = port.device

        # Skip Bluetooth and ACPI mother board ports
        if any(ign in hwid or ign in desc for ign in IGNORED_PATTERNS):
            continue

        score = 0
        device_label = "Serial Port"
        vid = port.vid
        pid = port.pid

        # Espressif native USB-Serial/JTAG (Direct ESP32-C3 / S3 connection)
        if vid == 0x303A:
            score += 100
            device_label = "Espressif USB-Serial/JTAG (ESP32-C3 SuperMini)"
        elif "esp32" in desc or "espressif" in desc or "jtag" in desc:
            score += 95
            device_label = "ESP32 Device"
        elif vid in KNOWN_ESP_VIDS:
            score += 70
            device_label = KNOWN_ESP_VIDS[vid]
        elif "usb serial" in desc or "usb-serial" in desc:
            score += 40
            device_label = "USB Serial Device"
        else:
            score += 10
            device_label = "Generic Serial Port"

        candidates.append({
            "port": device,
            "desc": port.description or "Unknown",
            "hwid": port.hwid or "",
            "vid": vid,
            "pid": pid,
            "score": score,
            "label": device_label,
            "is_esp": score >= 70
        })

    # Sort descending by ESP probability score
    candidates.sort(key=lambda x: x["score"], reverse=True)
    return candidates


def get_best_esp_port():
    """Returns the most likely ESP COM port string or None if none connected."""
    ports = scan_ports()
    if ports and ports[0]["score"] >= 40:
        return ports[0]
    return None


# =========================================================================
# FLASH RUNNER
# =========================================================================
def execute_flash(port, flash_type="factory", baud=460800, log_callback=None, progress_callback=None):
    """
    Executes esptool to flash the selected firmware image.
    Yields output lines and parses percentage for real-time progress.
    """
    def log(msg):
        if log_callback:
            log_callback(msg)
        else:
            print(msg)

    # Validate target files
    if flash_type == "factory":
        target_file = FACTORY_BIN
        offset = "0x0"
        extra_flags = ["--flash-mode", "dio", "--flash-size", "4MB"]
        name = "Full Factory Restore (Firmware + Mochi LittleFS 4MB)"
    elif flash_type == "app":
        target_file = FIRMWARE_BIN
        offset = "0x10000"
        extra_flags = []
        name = "App Firmware Only (0x10000, preserves settings)"
    elif flash_type == "fs":
        target_file = LITTLEFS_BIN
        offset = "0x170000"
        extra_flags = []
        name = "LittleFS Mochi Assets (0x170000)"
    else:
        log(f"[ERROR] Unknown flash type: {flash_type}")
        return False

    if not target_file.exists():
        log(f"[ERROR] Target binary not found: {target_file}")
        return False

    file_size_kb = target_file.stat().st_size / 1024.0
    log(f"\n=======================================================")
    log(f"⚡ FLASHING TARGET: {name}")
    log(f"📁 Binary: {target_file.name} ({file_size_kb:.1f} KB)")
    log(f"🔌 Serial Port: {port} @ {baud} baud")
    log(f"📍 Flash Offset: {offset}")
    log(f"=======================================================\n")

    cmd = [
        sys.executable, "-m", "esptool",
        "--chip", "esp32c3",
        "--port", port,
        "--baud", str(baud),
        "--before", "default-reset",
        "--after", "hard-reset",
        "write-flash",
    ] + extra_flags + [offset, str(target_file)]

    log(f"Executing: {' '.join(cmd)}\n")

    try:
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
            creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0
        )

        pct_pattern = re.compile(r"\((\d+)\s*%\)")

        for line in proc.stdout:
            clean_line = line.rstrip()
            if not clean_line:
                continue

            log(clean_line)

            # Extract percent progress
            match = pct_pattern.search(clean_line)
            if match and progress_callback:
                pct = int(match.group(1))
                progress_callback(pct)

        proc.wait()

        if proc.returncode == 0:
            if progress_callback:
                progress_callback(100)
            log("\n✅ SUCCESS: Flashing finished with zero errors!")
            log("🚀 ESP32-C3 reset triggered. Device is now running Spearhead OS v5.0!\n")
            return True
        else:
            log(f"\n❌ ERROR: esptool exited with error code {proc.returncode}")
            log("💡 Tip: If connection timed out, hold the BOOT button while plugging in the USB cable.\n")
            return False

    except Exception as e:
        log(f"\n❌ FATAL ERROR: {e}\n")
        return False


# =========================================================================
# CLI MODE
# =========================================================================
def run_cli(args):
    """Command-line interface execution."""
    print("====================================================")
    print("   SPEARHEAD // ESP32-C3 AUTO-FLASH UTILITY        ")
    print("====================================================")

    if args.list:
        ports = scan_ports()
        if not ports:
            print("No serial ports found.")
            return
        print(f"Found {len(ports)} candidate port(s):")
        for p in ports:
            tag = " [RECOMMENDED ESP]" if p["is_esp"] else ""
            print(f"  • {p['port']:<6} | {p['label']:<40} | VID={p['vid']} PID={p['pid']}{tag}")
        return

    port = args.port
    if not port:
        print("[SCAN] Scanning for connected ESP32-C3 devices...")
        best = get_best_esp_port()
        if best:
            port = best["port"]
            print(f"[DETECTED] Automatically selected {port} ({best['label']})")
        else:
            print("[WAIT] No ESP device found immediately. Waiting for device to be plugged in...")
            print("       (Connect your ESP32 keychain now, or press Ctrl+C to abort)")
            try:
                while not port:
                    time.sleep(1.0)
                    best = get_best_esp_port()
                    if best:
                        port = best["port"]
                        print(f"\n[DETECTED] Connected! Automatically selected {port} ({best['label']})")
                        break
            except KeyboardInterrupt:
                print("\nAborted.")
                sys.exit(0)

    # Proceed to flash
    success = execute_flash(port=port, flash_type=args.type, baud=args.baud)
    sys.exit(0 if success else 1)


# =========================================================================
# MODERN DESKTOP GUI (TKINTER)
# =========================================================================
def run_gui():
    """Launches the modern tactical dark-themed desktop application."""
    import tkinter as tk
    from tkinter import ttk, messagebox

    root = tk.Tk()
    root.title("SPEARHEAD // Firmware Flasher Studio")
    root.geometry("740x680")
    root.minsize(680, 580)
    root.configure(bg="#0B0F19")

    # Set Window Icon if present
    if ICON_PATH.exists():
        try:
            root.iconbitmap(str(ICON_PATH))
        except Exception:
            pass

    # Styling Constants
    COLOR_BG = "#0B0F19"
    COLOR_CARD = "#121826"
    COLOR_BORDER = "#1E293B"
    COLOR_ACCENT = "#00FF88"       # Tactical Emerald Green
    COLOR_CYAN = "#00E5FF"         # High-tech Cyan
    COLOR_AMBER = "#FFB703"        # Amber Warning
    COLOR_DANGER = "#FF0055"       # Neon Pink/Red
    COLOR_TEXT = "#F1F5F9"         # Primary White
    COLOR_MUTED = "#94A3B8"        # Secondary Slate
    COLOR_CONSOLE_BG = "#050811"   # Deep Terminal Black

    # UI State
    selected_port = tk.StringVar(value="")
    flash_type_var = tk.StringVar(value="factory")
    baud_rate_var = tk.StringVar(value="460800")
    auto_flash_var = tk.BooleanVar(value=False)
    is_flashing = False
    last_detected_port = None

    # Setup TTK Styles
    style = ttk.Style()
    style.theme_use("clam")
    style.configure("TFrame", background=COLOR_BG)
    style.configure("Card.TFrame", background=COLOR_CARD, relief="flat")
    style.configure("TLabel", background=COLOR_BG, foreground=COLOR_TEXT, font=("Segoe UI", 10))
    style.configure("Card.TLabel", background=COLOR_CARD, foreground=COLOR_TEXT, font=("Segoe UI", 10))
    style.configure("Muted.TLabel", background=COLOR_CARD, foreground=COLOR_MUTED, font=("Segoe UI", 9))
    style.configure("Header.TLabel", background=COLOR_BG, foreground=COLOR_ACCENT, font=("Segoe UI", 16, "bold"))
    style.configure("CardTitle.TLabel", background=COLOR_CARD, foreground=COLOR_CYAN, font=("Segoe UI", 11, "bold"))
    style.configure("TRadiobutton", background=COLOR_CARD, foreground=COLOR_TEXT, font=("Segoe UI", 10))
    style.configure("TCheckbutton", background=COLOR_CARD, foreground=COLOR_TEXT, font=("Segoe UI", 10))
    style.configure("TCombobox", fieldbackground=COLOR_CARD, background=COLOR_BORDER, foreground=COLOR_TEXT)
    style.configure(
        "Horizontal.TProgressbar",
        troughcolor=COLOR_BORDER,
        background=COLOR_ACCENT,
        thickness=18
    )

    # Main Scrollable / Padded Container
    main_frame = tk.Frame(root, bg=COLOR_BG, padx=22, pady=18)
    main_frame.pack(fill=tk.BOTH, expand=True)

    # 1. Header Banner
    header_frame = tk.Frame(main_frame, bg=COLOR_BG)
    header_frame.pack(fill=tk.X, pady=(0, 14))

    title_label = tk.Label(
        header_frame,
        text="⚡ SPEARHEAD // FIRMWARE FLASHER",
        font=("Segoe UI", 16, "bold"),
        fg=COLOR_ACCENT,
        bg=COLOR_BG
    )
    title_label.pack(anchor="w")

    subtitle_label = tk.Label(
        header_frame,
        text="Auto-Detecting Desktop Flasher for ESP32-C3 Tactical Keychain",
        font=("Segoe UI", 9),
        fg=COLOR_MUTED,
        bg=COLOR_BG
    )
    subtitle_label.pack(anchor="w")

    # 2. Port Detection Card
    card_port = tk.Frame(main_frame, bg=COLOR_CARD, bd=1, relief="solid", highlightbackground=COLOR_BORDER, highlightthickness=1)
    card_port.pack(fill=tk.X, pady=(0, 12), ipady=8, ipadx=10)

    port_title_frame = tk.Frame(card_port, bg=COLOR_CARD)
    port_title_frame.pack(fill=tk.X, padx=12, pady=(6, 4))

    lbl_port_title = tk.Label(port_title_frame, text="CONNECTED HARDWARE", font=("Segoe UI", 10, "bold"), fg=COLOR_CYAN, bg=COLOR_CARD)
    lbl_port_title.pack(side=tk.LEFT)

    lbl_port_status = tk.Label(port_title_frame, text="● SCANNING...", font=("Segoe UI", 9, "bold"), fg=COLOR_AMBER, bg=COLOR_CARD)
    lbl_port_status.pack(side=tk.RIGHT)

    port_controls = tk.Frame(card_port, bg=COLOR_CARD)
    port_controls.pack(fill=tk.X, padx=12, pady=(4, 6))

    combo_ports = ttk.Combobox(port_controls, textvariable=selected_port, state="readonly", width=42, font=("Segoe UI", 10))
    combo_ports.pack(side=tk.LEFT, padx=(0, 10))

    btn_rescan = tk.Button(
        port_controls,
        text="🔄 Rescan Ports",
        font=("Segoe UI", 9, "bold"),
        bg="#1E293B",
        fg=COLOR_TEXT,
        activebackground="#334155",
        activeforeground=COLOR_TEXT,
        bd=0,
        padx=12,
        pady=5,
        cursor="hand2",
        command=lambda: scan_and_update_ui(manual=True)
    )
    btn_rescan.pack(side=tk.LEFT)

    lbl_device_info = tk.Label(
        card_port,
        text="Plug in your ESP32-C3 via USB. Port will be chosen automatically.",
        font=("Segoe UI", 9),
        fg=COLOR_MUTED,
        bg=COLOR_CARD
    )
    lbl_device_info.pack(anchor="w", padx=12, pady=(2, 6))

    # 3. Flashing Options Card
    card_opt = tk.Frame(main_frame, bg=COLOR_CARD, bd=1, relief="solid", highlightbackground=COLOR_BORDER, highlightthickness=1)
    card_opt.pack(fill=tk.X, pady=(0, 12), ipady=8, ipadx=10)

    lbl_opt_title = tk.Label(card_opt, text="FIRMWARE TARGET SELECTION", font=("Segoe UI", 10, "bold"), fg=COLOR_CYAN, bg=COLOR_CARD)
    lbl_opt_title.pack(anchor="w", padx=12, pady=(6, 8))

    r1 = tk.Radiobutton(
        card_opt,
        text="Full Factory Restore (4MB) — Firmware + Bootloader + Partitions + 43 Mochi GIFs",
        variable=flash_type_var,
        value="factory",
        bg=COLOR_CARD,
        fg=COLOR_TEXT,
        activebackground=COLOR_CARD,
        activeforeground=COLOR_ACCENT,
        selectcolor="#0B0F19",
        font=("Segoe UI", 9, "bold")
    )
    r1.pack(anchor="w", padx=16, pady=2)

    r2 = tk.Radiobutton(
        card_opt,
        text="App Firmware Only (0x10000) — Fast 4-second update, keeps LittleFS & NVS settings",
        variable=flash_type_var,
        value="app",
        bg=COLOR_CARD,
        fg=COLOR_TEXT,
        activebackground=COLOR_CARD,
        activeforeground=COLOR_ACCENT,
        selectcolor="#0B0F19",
        font=("Segoe UI", 9)
    )
    r2.pack(anchor="w", padx=16, pady=2)

    r3 = tk.Radiobutton(
        card_opt,
        text="LittleFS Assets Only (0x170000) — Re-upload Dasai Mochi GIF collection only",
        variable=flash_type_var,
        value="fs",
        bg=COLOR_CARD,
        fg=COLOR_TEXT,
        activebackground=COLOR_CARD,
        activeforeground=COLOR_ACCENT,
        selectcolor="#0B0F19",
        font=("Segoe UI", 9)
    )
    r3.pack(anchor="w", padx=16, pady=2)

    # Sub-row: Baud Rate & Auto-Flash
    sub_row = tk.Frame(card_opt, bg=COLOR_CARD)
    sub_row.pack(fill=tk.X, padx=16, pady=(8, 4))

    tk.Label(sub_row, text="Baud Rate:", font=("Segoe UI", 9), fg=COLOR_MUTED, bg=COLOR_CARD).pack(side=tk.LEFT, padx=(0, 6))
    combo_baud = ttk.Combobox(sub_row, textvariable=baud_rate_var, values=["460800", "921600", "115200"], width=10, state="readonly")
    combo_baud.pack(side=tk.LEFT, padx=(0, 20))

    chk_auto = tk.Checkbutton(
        sub_row,
        text="⚡ Auto-Flash immediately when ESP is plugged in",
        variable=auto_flash_var,
        bg=COLOR_CARD,
        fg=COLOR_TEXT,
        activebackground=COLOR_CARD,
        activeforeground=COLOR_ACCENT,
        selectcolor="#0B0F19",
        font=("Segoe UI", 9)
    )
    chk_auto.pack(side=tk.LEFT)

    # 4. Action & Progress Area
    action_frame = tk.Frame(main_frame, bg=COLOR_BG)
    action_frame.pack(fill=tk.X, pady=(0, 10))

    btn_flash = tk.Button(
        action_frame,
        text="⚡ FLASH FIRMWARE NOW",
        font=("Segoe UI", 12, "bold"),
        bg=COLOR_ACCENT,
        fg="#050811",
        activebackground="#00CC6A",
        activeforeground="#050811",
        bd=0,
        pady=10,
        cursor="hand2",
        command=lambda: start_flash_thread()
    )
    btn_flash.pack(fill=tk.X)

    prog_frame = tk.Frame(action_frame, bg=COLOR_BG)
    prog_frame.pack(fill=tk.X, pady=(10, 0))

    prog_bar = ttk.Progressbar(prog_frame, orient="horizontal", mode="determinate", style="Horizontal.TProgressbar")
    prog_bar.pack(fill=tk.X)

    lbl_pct = tk.Label(prog_frame, text="Ready to flash", font=("Segoe UI", 9), fg=COLOR_MUTED, bg=COLOR_BG)
    lbl_pct.pack(anchor="w", pady=(3, 0))

    # 5. Live Console Log
    console_frame = tk.Frame(main_frame, bg=COLOR_CARD, bd=1, relief="solid", highlightbackground=COLOR_BORDER, highlightthickness=1)
    console_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 4))

    console_header = tk.Frame(console_frame, bg=COLOR_CARD)
    console_header.pack(fill=tk.X, padx=10, pady=(6, 4))

    lbl_con_title = tk.Label(console_header, text="TERMINAL OUTPUT", font=("Segoe UI", 9, "bold"), fg=COLOR_MUTED, bg=COLOR_CARD)
    lbl_con_title.pack(side=tk.LEFT)

    btn_clear = tk.Button(
        console_header,
        text="Clear",
        font=("Segoe UI", 8),
        bg="#1E293B",
        fg=COLOR_MUTED,
        activebackground="#334155",
        activeforeground=COLOR_TEXT,
        bd=0,
        padx=8,
        pady=2,
        cursor="hand2",
        command=lambda: clear_console()
    )
    btn_clear.pack(side=tk.RIGHT)

    text_console = tk.Text(
        console_frame,
        bg=COLOR_CONSOLE_BG,
        fg="#38BDF8",
        font=("Consolas", 9),
        bd=0,
        highlightthickness=0,
        padx=10,
        pady=8,
        wrap="none"
    )
    scroll_y = tk.Scrollbar(console_frame, orient=tk.VERTICAL, command=text_console.yview)
    text_console.configure(yscrollcommand=scroll_y.set)

    scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
    text_console.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

    text_console.tag_configure("success", foreground=COLOR_ACCENT)
    text_console.tag_configure("error", foreground=COLOR_DANGER)
    text_console.tag_configure("amber", foreground=COLOR_AMBER)
    text_console.tag_configure("cyan", foreground=COLOR_CYAN)

    def append_log(msg):
        text_console.insert(tk.END, msg + "\n")
        text_console.see(tk.END)

    def clear_console():
        text_console.delete("1.0", tk.END)

    # =========================================================================
    # PORT POLLING & AUTO-DETECTION CONTROLLER
    # =========================================================================
    def scan_and_update_ui(manual=False):
        nonlocal last_detected_port
        if is_flashing:
            return

        ports = scan_ports()
        port_list = [f"{p['port']} - {p['label']}" for p in ports]
        combo_ports["values"] = port_list

        best = ports[0] if (ports and ports[0]["score"] >= 40) else None

        if best:
            best_str = f"{best['port']} - {best['label']}"
            lbl_port_status.config(text=f"● CONNECTED ({best['port']})", fg=COLOR_ACCENT)
            lbl_device_info.config(text=f"Target: {best['label']} (HWID: {best['hwid'][:45]}...)", fg=COLOR_TEXT)
            btn_flash.config(state=tk.NORMAL, bg=COLOR_ACCENT)

            # Auto select if port changed or not set
            if selected_port.get() not in port_list or (manual and not selected_port.get()):
                selected_port.set(best_str)

            # Trigger auto-flash if enabled and newly plugged in
            if auto_flash_var.get() and last_detected_port != best['port'] and not is_flashing:
                last_detected_port = best['port']
                append_log(f"\n[AUTO-DETECT] New ESP32 plugged in on {best['port']}! Initiating Auto-Flash...")
                root.after(300, start_flash_thread)
            else:
                last_detected_port = best['port']

        else:
            lbl_port_status.config(text="○ WAITING FOR DEVICE...", fg=COLOR_AMBER)
            lbl_device_info.config(text="No ESP32 detected. Plug in your keychain USB cable to auto-connect.", fg=COLOR_MUTED)
            last_detected_port = None
            if not manual and not selected_port.get():
                btn_flash.config(state=tk.DISABLED, bg="#334155")

        # Re-schedule background scan every 1000ms
        root.after(1000, scan_and_update_ui)

    def update_progress(pct):
        prog_bar["value"] = pct
        lbl_pct.config(text=f"Flashing memory... {pct}%", fg=COLOR_ACCENT)
        root.update_idletasks()

    # =========================================================================
    # FLASHING THREAD LAUNCHER
    # =========================================================================
    def start_flash_thread():
        nonlocal is_flashing
        if is_flashing:
            return

        # Extract COM port from combobox selection (e.g. "COM7 - Espressif...")
        sel = selected_port.get().strip()
        if not sel:
            messagebox.showwarning("No Port Selected", "Please plug in your ESP32 or select a COM port.")
            return

        port = sel.split(" ")[0].strip()

        is_flashing = True
        btn_flash.config(state=tk.DISABLED, text="⏳ FLASHING IN PROGRESS...", bg="#334155")
        btn_rescan.config(state=tk.DISABLED)
        prog_bar["value"] = 0
        lbl_pct.config(text="Connecting to ESP32 ROM bootloader...", fg=COLOR_CYAN)

        def worker():
            nonlocal is_flashing
            try:
                baud = int(baud_rate_var.get())
            except ValueError:
                baud = 460800

            success = execute_flash(
                port=port,
                flash_type=flash_type_var.get(),
                baud=baud,
                log_callback=append_log,
                progress_callback=update_progress
            )

            is_flashing = False
            root.after(0, lambda: on_flash_complete(success))

        threading.Thread(target=worker, daemon=True).start()

    def on_flash_complete(success):
        btn_flash.config(state=tk.NORMAL, text="⚡ FLASH FIRMWARE NOW", bg=COLOR_ACCENT)
        btn_rescan.config(state=tk.NORMAL)
        if success:
            lbl_pct.config(text="✅ Flash Completed Successfully! Device rebooted.", fg=COLOR_ACCENT)
            messagebox.showinfo("Flash Success", "Firmware was flashed successfully!\n\nYour ESP32-C3 is now running Spearhead OS.")
        else:
            lbl_pct.config(text="❌ Flash Failed. Check terminal output for details.", fg=COLOR_DANGER)
            messagebox.showerror("Flash Failed", "Flashing failed.\n\nPlease check the terminal log and ensure the port is not open in another serial monitor.")

    # Initial Scan
    append_log("SPEARHEAD Desktop Firmware Flasher Studio initialized.")
    append_log(f"Firmware Binary: {FACTORY_BIN.name} ({FACTORY_BIN.stat().st_size/1024:.1f} KB)")
    scan_and_update_ui(manual=True)

    root.mainloop()


# =========================================================================
# ENTRY POINT
# =========================================================================
def main():
    parser = argparse.ArgumentParser(
        description="SPEARHEAD // ESP32-C3 Firmware Flashing Studio & CLI Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python flash_tool.py                  # Launch Modern Desktop GUI
  python flash_tool.py --auto           # Auto-scan and flash immediately without prompts
  python flash_tool.py --list           # List all detected serial ports and ESP devices
  python flash_tool.py --port COM7      # Flash specific COM port
  python flash_tool.py --type app       # Flash app firmware only (faster, preserves LittleFS)
        """
    )
    parser.add_argument("--auto", action="store_true", help="Auto-detect port and flash immediately with default settings")
    parser.add_argument("--port", "-p", type=str, help="Specify COM port (e.g. COM7)")
    parser.add_argument("--baud", "-b", type=int, default=460800, help="Flashing baud rate (default: 460800)")
    parser.add_argument("--type", "-t", choices=["factory", "app", "fs"], default="factory",
                        help="Target to flash: factory (full 4MB), app (firmware only), fs (LittleFS GIFs only)")
    parser.add_argument("--list", "-l", action="store_true", help="List all candidate serial ports and exit")
    parser.add_argument("--cli", action="store_true", help="Force command-line interface instead of GUI")
    parser.add_argument("--gui", action="store_true", help="Force GUI desktop window")

    args = parser.parse_args()

    # If any CLI action flags are specified, run in CLI mode
    if args.auto or args.list or args.port or args.cli:
        run_cli(args)
    else:
        run_gui()


if __name__ == "__main__":
    main()

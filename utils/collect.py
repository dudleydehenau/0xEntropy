"""0xEntropy Stream Dumper - Acquisition du flux binaire brut via USB CDC

Example:
    python dump_entropy.py -p COM4 -s 100000000 -o random.bin //windows
    python dump_entropy.py -p /dev/ttyACM0 -s 10000000000 -o dump_10G.bin /linux&Mac
"""

import argparse
import os
import sys
import time
import serial

DEFAULT_PORT = "COM4"
DEFAULT_BAUD = 115200
DEFAULT_TARGET = 10_000_000_000  # 10 Go
DEFAULT_OUTPUT = "random_data.bin"
CHUNK_SIZE = 4096


def collect(port, baudrate, target_bytes, output_path):
    collected = 0
    if os.path.exists(output_path):
        collected = os.path.getsize(output_path)

    if collected >= target_bytes:
        print(f"[!] Fichier déjà complet ({collected}/{target_bytes} bytes).")
        return

    print(f"[*] Ouverture de {port} à {baudrate} baud...")
    try:
        ser = serial.Serial(port, baudrate, timeout=1.0)
    except serial.SerialException as e:
        sys.exit(f"[-] Erreur port série : {e}")

    if collected > 0:
        print(f"[*] Reprise à {collected} bytes ({target_bytes - collected} restants)")

    start_time = time.time()
    last_ui_update = start_time
    last_bytes_count = 0

    try:
        with open(output_path, "ab") as f:
            while collected < target_bytes:
                chunk = ser.read(min(CHUNK_SIZE, target_bytes - collected))
                if not chunk:
                    continue

                f.write(chunk)
                collected += len(chunk)
                last_bytes_count += len(chunk)

                now = time.time()
                dt = now - last_ui_update

                if dt >= 1.0:
                    speed_kbps = (last_bytes_count * 8) / (dt * 1000)
                    progress = (collected / target_bytes) * 100
                    elapsed = now - start_time

                    print(
                        f"\r[{progress:6.2f}%] {collected}/{target_bytes} B | "
                        f"{speed_kbps:8.2f} kbps | {elapsed:.0f}s",
                        end="",
                        flush=True,
                    )

                    f.flush()
                    last_bytes_count = 0
                    last_ui_update = now

        print(f"\n[+] Collecte terminée ({collected} bytes en {time.time() - start_time:.1f}s)")

    except KeyboardInterrupt:
        print(f"\n[!] Interruption. Total sauvegardé : {collected} bytes.")

    finally:
        ser.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Acquisition du flux brut 0xEntropy")
    parser.add_argument("-p", "--port", default=DEFAULT_PORT, help="Port série (ex: COM4, /dev/ttyACM0)")
    parser.add_argument("-b", "--baud", type=int, default=DEFAULT_BAUD, help="Baudrate")
    parser.add_argument("-s", "--size", type=int, default=DEFAULT_TARGET, help="Taille cible en octets")
    parser.add_argument("-o", "--output", default=DEFAULT_OUTPUT, help="Fichier de sortie")

    args = parser.parse_args()
    collect(args.port, args.baud, args.size, args.output)
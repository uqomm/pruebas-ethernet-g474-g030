#!/usr/bin/env python3
"""
W5500 UDP Test Script — STM32G474 Ethernet Verification
Auto-detects best bind IP. Works even if link not yet up.
Usage: python test_w5500.py
"""

import socket
import time
import sys

STM32_IP = "192.168.1.50"
PORT = 5000
TIMEOUT = 5.0

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    for bind_ip in ("192.168.1.10", "0.0.0.0"):
        try:
            sock.bind((bind_ip, PORT))
            print(f"[OK] Bound to {bind_ip}:{PORT}")
            break
        except OSError:
            continue
    else:
        print("ERROR: Cannot bind UDP port 5000")
        sys.exit(1)

    sock.settimeout(TIMEOUT)
    print(f"[INFO] Waiting for keep-alive from {STM32_IP}...")
    print(f"[INFO] Press Ctrl+C to stop\n")

    pkt = 0
    sent = False
    try:
        while True:
            try:
                data, addr = sock.recvfrom(2048)
                pkt += 1
                msg = data.decode(errors='replace').rstrip()
                print(f"[RX #{pkt} from {addr[0]}:{addr[1]}] {msg}")
                if not sent:
                    test = f"PC_TEST_OK:{time.ctime()}"
                    sock.sendto(test.encode(), (STM32_IP, PORT))
                    print(f"[TX -> {STM32_IP}:{PORT}] {test}")
                    print(f"[INFO] Waiting for echo back...")
                    sent = True
            except socket.timeout:
                if not sent:
                    sock.sendto(b"PING_FROM_PC", (STM32_IP, PORT))
                    print(f"[TX -> {STM32_IP}:{PORT}] PING_FROM_PC")
    except KeyboardInterrupt:
        print(f"\n[DONE] Received {pkt} packets.")
        print(f"[RESULT] {'PASS' if pkt > 0 else 'FAIL - Check wiring/IP/cable'}")

if __name__ == "__main__":
    main()

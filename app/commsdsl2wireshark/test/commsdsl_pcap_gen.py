import struct
import time

def commsdsl_write_pcap_header(f):
    # Global Header
    # magic_number (4), version_major (2), version_minor (2), thiszone (4), sigfigs (4), snaplen (4), network (4)
    # network 1 = Ethernet
    # network 101 = Raw IP ? Let's use 1 (Ethernet) and fake ethernet headers for simplicity,
    # or use 147 (USER 0) to avoid needing ethernet headers if we just assume raw payload.
    # Let's use Ethernet (1) as it's standard.
    encoded = struct.pack('IHHIIII', 0xa1b2c3d4, 2, 4, 0, 0, 65535, 1)
    f.write(encoded)

def commsdsl_write_packet(f, data, timestamp):
    # Packet Header
    # ts_sec (4), ts_usec (4), incl_len (4), orig_len (4)
    ts_sec = int(timestamp)
    ts_usec = int((timestamp - ts_sec) * 1000000)
    length = len(data)

    encoded = struct.pack('IIII', ts_sec, ts_usec, length, length)
    f.write(encoded)
    f.write(data)

def commsdsl_create_ethernet_ip_tcp_headers(payload_len, seq_num=1):
    # Simple fake headers to make it valid Ethernet/IP/TCP
    # This is quite verbose to implement manually without scapy.
    # Alternative: Use LinkType 147 (USER0) to write raw protocol data if we register the dissector to a USER DLT?
    # No, the user wants a standard plugin usually bound to a port.
    # Let's bind to TCP port 8888.

    # Validation: writing raw UDP or TCP frames manually in python struct is error prone.
    # Let's try to minimalize:
    # 1. Ethernet Header (14 bytes)
    # 2. IP Header (20 bytes)
    # 3. TCP Header (20 bytes)

    # Ether
    # Dest: 00:00:00:00:00:00
    # Src:  00:00:00:00:00:00
    # Type: 0x0800 (IPv4)
    eth = b'\x00'*12 + b'\x08\x00'

    # IP
    # Ver/IHL (1), TOS (1), TotLen (2), ID (2), Frag (2), TTL (1), Proto (1), Checksum (2), Src (4), Dst (4)
    # Proto 6 = TCP
    total_len = 20 + 20 + payload_len
    ip_header = struct.pack('!BBHHHBBH4s4s',
                            0x45, 0, total_len, 0, 0, 64, 6, 0,
                            b'\x7f\x00\x00\x01', b'\x7f\x00\x00\x01') # 127.0.0.1

    # Checksum calculation (skip for loopback usually, or simple sum)
    # We can leave 0 for some readers, but Wireshark might complain if validating checksums is on.

    # TCP
    # SrcPort (2), DstPort (2), Seq (4), Ack (4), Offset (1), Flags (1), Window (2), Checksum (2), UrgPtr (2)
    # DstPort = 8888
    tcp_header = struct.pack('!HHIIBBHHH',
                             12345, 8888, seq_num, 1, (5 << 4), 0x18, 8192, 0, 0) # PSH+ACK

    return eth + ip_header + tcp_header
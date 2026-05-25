import sys

from commsdsl_pcap_gen import *

def do_frame_short(sync, size, id, payload):
    prefix = struct.pack('>HBB', sync, size, id)
    return prefix + payload

def do_frame_long(sync, size, id, payload):
    prefix = struct.pack('>HBHB', sync, 0xff, size, id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>BBBBBH', 0, 1, 2, 1, 2, 3)
    msg1 = do_frame_short(0xabcd, 1 + len(msg1_payload), 1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('>BH3sB', 0xff, 3, b"bla", 0x11)
    msg2 = do_frame_long(0xabcd, 1 + len(msg2_payload), 2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload = struct.pack('>B3s', 3, b"\x01\x02\x03")
    msg3 = do_frame_long(0xabcd, 1 + len(msg3_payload), 3, msg3_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3), seq)
    commsdsl_write_packet(f, header + msg3, time.time())

def pcap4(f):
    seq = 4000
    msg4_payload = struct.pack('>BHHH', 0xff, 4, 1, 2)
    msg4 = do_frame_short(0xabcd, 1 + len(msg4_payload), 4, msg4_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg4), seq)
    commsdsl_write_packet(f, header + msg4, time.time())

def pcap5(f):
    seq = 5000
    msg5_payload = struct.pack('>HB3sBH3s', 10 | (0x1 << 14), 3, b"bla", 0xff, 3, b"abc")
    msg5 = do_frame_long(0xabcd, 1 + len(msg5_payload), 5, msg5_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg5), seq)
    commsdsl_write_packet(f, header + msg5, time.time())

def pcap6(f):
    seq = 6000
    msg6_payload = struct.pack('>H3s', 3 | (0xf << 12), b"bla")
    msg6 = do_frame_short(0xabcd, 1 + len(msg6_payload), 6, msg6_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg6), seq)
    commsdsl_write_packet(f, header + msg6, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)
        pcap3(f)
        pcap4(f)
        pcap5(f)
        pcap6(f)

if __name__ == '__main__':
    main()
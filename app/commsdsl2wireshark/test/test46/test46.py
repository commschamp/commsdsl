from functools import reduce
import operator
import sys

from commsdsl_pcap_gen import *

def do_frame_sum8(sync, size, id, payload, checksum):
    prefix = struct.pack('>HHBB', sync, size, id, 0)
    suffix = struct.pack('>B', checksum)
    return prefix + payload + suffix

def do_frame_crc16(sync, size, id, payload, checksum):
    prefix = struct.pack('>HHBB', sync, size, id, 1)
    suffix = struct.pack('>H', checksum)
    return prefix + payload + suffix

def do_frame_crc32(sync, size, id, payload, checksum):
    prefix = struct.pack('>HHBB', sync, size, id, 2)
    suffix = struct.pack('>I', checksum)
    return prefix + payload + suffix

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>H', 0x1234)
    msg1 = do_frame_sum8(0xabcd, 2 + len(msg1_payload), 1, msg1_payload, 195)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('>I', 0x12345678)
    msg2 = do_frame_crc16(0xabcd, 2 + len(msg2_payload), 2, msg2_payload, 0xe45e)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload = struct.pack('>B3s', 3, b"bla")
    msg3 = do_frame_crc32(0xabcd, 2 + len(msg3_payload), 3, msg3_payload, 0xce25ac7e)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3), seq)
    commsdsl_write_packet(f, header + msg3, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)
        pcap3(f)

if __name__ == '__main__':
    main()
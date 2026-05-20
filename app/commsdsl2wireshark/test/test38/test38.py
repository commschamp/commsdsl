from functools import reduce
import operator
import sys

from commsdsl_pcap_gen import *

def do_frame(size, id, flags, payload):
    prefix = struct.pack('>HBB', size, id, flags)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('')
    msg1 = do_frame(2 + len(msg1_payload), 1, 0x0, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('>BB', 1, 3)
    msg2 = do_frame(2 + len(msg2_payload), 2, 0x23, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload = struct.pack('')
    msg3_1 = do_frame(2 + len(msg3_payload), 3, 0x21, msg3_payload)
    msg3_2 = do_frame(2 + len(msg3_payload), 3, 0x13, msg3_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3_1) + len(msg3_2), seq)
    commsdsl_write_packet(f, header + msg3_1 + msg3_2, time.time())

def pcap4(f):
    seq = 4000
    msg4_payload = struct.pack('')
    msg4_1 = do_frame(2 + len(msg4_payload), 4, 0x12, msg4_payload)
    msg4_2 = do_frame(2 + len(msg4_payload), 4, 0x22, msg4_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg4_1) + len(msg4_2), seq)
    commsdsl_write_packet(f, header + msg4_1 + msg4_2, time.time())

def pcap5(f):
    seq = 5000
    msg5_payload = struct.pack('>BB', 1, 1)
    msg5_1 = do_frame(2 + len(msg5_payload), 5, 0x01, msg5_payload)
    msg5_2 = do_frame(2 + len(msg5_payload), 5, 0x02, msg5_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg5_1) + len(msg5_2), seq)
    commsdsl_write_packet(f, header + msg5_1 + msg5_2, time.time())

def pcap6(f):
    seq = 6000
    msg6_payload = struct.pack('>BB', 1, 1)
    msg6 = do_frame(2 + len(msg6_payload), 6, 0x03, msg6_payload)
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
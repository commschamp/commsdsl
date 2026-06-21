import sys

from commsdsl_pcap_gen import *

def do_frame(id, version, payload):
    prefix = struct.pack('>BB', id, version)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload1 = struct.pack('>BBBH', 1, 1, 2, 0xabcd)
    msg1_1 = do_frame(1, 3, msg1_payload1)
    msg1_payload2 = struct.pack('>BBBIH', 1, 5, 2, 3, 0xabcd)
    msg1_2 = do_frame(1, 5, msg1_payload2)
    msg1_payload3 = struct.pack('>BBBIH', 1, 5, 2, 3, 0xabcd)
    msg1_3 = do_frame(1, 4, msg1_payload3)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1_1) + len(msg1_2) + len(msg1_3), seq)
    commsdsl_write_packet(f, header + msg1_1 + msg1_2 + msg1_3, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
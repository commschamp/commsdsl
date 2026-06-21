import sys

from commsdsl_pcap_gen import *

def do_frame(sync1, id, payload, sync2):
    prefix = struct.pack('>BB', sync1, id)
    suffix = struct.pack('>B', sync2)
    return prefix + payload + suffix

def pcap1(f):
    seq = 1000
    msg1_garbage = struct.pack('>3B', 0x11, 0x22, 0x33)
    msg1_payload = struct.pack('>3B', 0x01, 0x10, 0x02)
    msg1 = do_frame(2, 0, msg1_payload, 3)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1_garbage) + len(msg1), seq)
    commsdsl_write_packet(f, header + msg1_garbage + msg1, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
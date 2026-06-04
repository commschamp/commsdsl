import sys

from commsdsl_pcap_gen import *

def do_frame(sync1, size, id, payload, sync2):
    prefix = struct.pack('>BHB', sync1, size, id)
    suffix = struct.pack('>B', sync2)
    return prefix + payload + suffix

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>H', 0x0102)
    msg1 = do_frame(0x2, 2 + len(msg1_payload), 0, msg1_payload, 0x3)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
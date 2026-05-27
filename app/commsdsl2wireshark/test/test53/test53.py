import sys

from commsdsl_pcap_gen import *

def do_sub1_frame(size, id, payload):
    prefix = struct.pack('>HB', size, id)
    return prefix + payload

def do_sub2_frame(size, id, payload):
    return do_sub1_frame(size, id, payload)

def pcap1(f):
    seq = 1000
    sub1_msg1_payload = struct.pack('>H', 1)
    sub1_msg1 = do_sub1_frame(1 + len(sub1_msg1_payload), 1, sub1_msg1_payload)
    sub2_msg2_payload = struct.pack('>I', 2)
    sub2_msg2 = do_sub2_frame(1 + len(sub2_msg2_payload), 2, sub2_msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(sub1_msg1) + len(sub2_msg2), seq)
    commsdsl_write_packet(f, header + sub1_msg1 + sub2_msg2, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
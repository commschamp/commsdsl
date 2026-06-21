import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('>B', id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>BH3s', 0xff, 3, b"bla")
    msg1 = do_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg1_payload = struct.pack('>B3s', 3, b"abc")
    msg1 = do_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)

if __name__ == '__main__':
    main()
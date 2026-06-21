import sys

from commsdsl_pcap_gen import *

def do_frame(size, id, payload):
    prefix = struct.pack('>HB', size, id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>BBfBB3s', 2, 2, 1.23, 5, 3, b"bla")
    msg1 = do_frame(1 + len(msg1_payload), 1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('>BH', 0, 1)
    msg2 = do_frame(1 + len(msg2_payload), 2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)

if __name__ == '__main__':
    main()
import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('>B', id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg_payload = struct.pack('>B', 1)
    msg = do_frame(1, msg_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg), seq)
    commsdsl_write_packet(f, header + msg, time.time())

def pcap2(f):
    seq = 2000
    msg_payload = struct.pack('>B', 2)
    msg = do_frame(2, msg_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg), seq)
    commsdsl_write_packet(f, header + msg, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)

if __name__ == '__main__':
    main()
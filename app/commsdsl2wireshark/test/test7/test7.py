import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('!B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    msg1_payload = struct.pack('>HBB', 0, 0x06, 0x08)
    msg1 = do_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
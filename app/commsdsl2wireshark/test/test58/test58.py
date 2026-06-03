import sys

from commsdsl_pcap_gen import *

def do_frame(sync, size, checksum, id, payload):
    prefix = struct.pack('>HHBB', sync, size, checksum, id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>3s', b"\x01\02\03")
    msg1 = do_frame(0xabcd, 2 + len(msg1_payload), 0x06, 0, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
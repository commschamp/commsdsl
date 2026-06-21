from functools import reduce
import operator
import sys

from commsdsl_pcap_gen import *

def do_frame(sync, size, id, payload, checksum):
    prefix = struct.pack('>HHB', sync, size, id)
    suffix = struct.pack('>H', checksum)
    return prefix + payload + suffix

def calc_checksum(size, id, payload):
    prefix = struct.pack('>HB', size, id)
    data = prefix + payload
    return reduce(operator.xor, data)

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>B', 1)
    msg1_checksum = calc_checksum(6, 0, msg1_payload)
    msg1 = do_frame(0xabcd, 6, 0, msg1_payload, msg1_checksum)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
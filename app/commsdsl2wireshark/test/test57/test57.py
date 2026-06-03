import sys

from commsdsl_pcap_gen import *

def do_frame(sync1, size, id, payload, sync2):
    prefix = struct.pack('>HHB', sync1, size, id)
    suffix = struct.pack('>H', sync2)
    return prefix + payload + suffix

def pcap1(f):
    seq = 1000
    msg2_payload = struct.pack('>3s', b"bla")
    msg2 = do_frame(0x3d3d, 3 + len(msg2_payload), 1, msg2_payload, 0x4040)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
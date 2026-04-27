import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('!B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    payload = struct.pack('>5IB2IBBIBIBBII', *([0x11111111] * 5), 2, *([0x22222222] * 2), 10, *([4, 0x33333333] * 2), 9, 4, *([0x44444444] * 2))
    msg1 = do_frame(1, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    payload1 = struct.pack('>6s4sB', b"hello\x00", b"bla\x00", 0)
    payload2 = struct.pack('>2s2sB', b"a\x00", b"b\x00", 0)
    msg2_1 = do_frame(2, payload1)
    msg2_2 = do_frame(2, payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2_1) + len(msg2_2), seq)
    commsdsl_write_packet(f, header + msg2_1 + msg2_2, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)

if __name__ == '__main__':
    main()
import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('!B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    payload = struct.pack('>BBHHBBBIBBB', 0, 1, 2, 3, 1, 3, 5, 0x12345678, 6, 7, 8)
    msg1 = do_frame(1, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    payload = struct.pack('>BBBBBBB', 1, 2, 1, 3, 4, 5, 6)
    msg2 = do_frame(2, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    payload = struct.pack('>B', 0)
    msg3 = do_frame(3, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3), seq)
    commsdsl_write_packet(f, header + msg3, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)
        pcap3(f)

if __name__ == '__main__':
    main()
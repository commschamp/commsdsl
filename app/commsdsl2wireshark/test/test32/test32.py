import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('>B', id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload = struct.pack('>BBIH', 4, 4, 0, 0xffff)
    msg1 = do_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('>BBBHBH', 1, 1, 2, 1, 2, 1)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload1 = struct.pack('>BBHH', 0, 2, 1, 0xabcd)
    msg3_1 = do_frame(3, msg3_payload1)
    msg3_payload2 = struct.pack('>BBH300sH', 5, 0xff, 300, b'a' * 300, 0xabcd)
    msg3_2 = do_frame(3, msg3_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3_1) + len(msg3_2), seq)
    commsdsl_write_packet(f, header + msg3_1 + msg3_2, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)
        pcap3(f)

if __name__ == '__main__':
    main()
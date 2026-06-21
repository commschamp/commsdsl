import sys

from commsdsl_pcap_gen import *

def do_frame(id, version, payload):
    prefix = struct.pack('>BB', id, version)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload1 = struct.pack('>4B6I', 2, 4, 3, 4, 1, 2, 3, 4, 5, 6)
    msg1_1 = do_frame(1, 5, msg1_payload1)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1_1), seq)
    commsdsl_write_packet(f, header + msg1_1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload1 = struct.pack('>BBIIIBBIIII', 2, 4, 1, 2, 3, 2, 8, 5, 6, 7 ,8)
    msg2_1 = do_frame(2, 5, msg2_payload1)
    msg2_payload2 = struct.pack('>BBIII', 2, 4, 1, 2, 3)
    msg2_2 = do_frame(2, 2, msg2_payload2)
    msg2_payload3 = struct.pack('>BBII', 2, 4, 1, 2)
    msg2_3 = do_frame(2, 1, msg2_payload3)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2_1) + len(msg2_2) + len(msg2_3), seq)
    commsdsl_write_packet(f, header + msg2_1 + msg2_2 + msg2_3, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload1 = struct.pack('>BII', 0xf2, 1, 2)
    msg3_1 = do_frame(3, 5, msg3_payload1)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3_1), seq)
    commsdsl_write_packet(f, header + msg3_1, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)
        pcap3(f)

if __name__ == '__main__':
    main()
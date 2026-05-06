import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('>B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    msg1_payload1 = struct.pack('>B', 1)
    msg1_1 = do_frame(1, msg1_payload1)
    msg1_payload2 = struct.pack('>BH', 0, 5)
    msg1_2 = do_frame(1, msg1_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1_1) + len(msg1_2), seq)
    commsdsl_write_packet(f, header + msg1_1 + msg1_2, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload1 = struct.pack('>B', 1)
    msg2_1 = do_frame(2, msg2_payload1)
    msg2_payload2 = struct.pack('>BH', 5, 5)
    msg2_2 = do_frame(2, msg2_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2_1) + len(msg2_2), seq)
    commsdsl_write_packet(f, header + msg2_1 + msg2_2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload1 = struct.pack('>8s', b"bla\x00\x00\x00\x00\x00")
    msg3_1 = do_frame(3, msg3_payload1)
    msg3_payload2 = struct.pack('>8sH', b"hello\x00\x00\x00", 5)
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
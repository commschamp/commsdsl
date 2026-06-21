import sys

from commsdsl_pcap_gen import *

def do_frame(size, id, payload):
    prefix = struct.pack('>BB', size, id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload1 = struct.pack('>B', 1)
    msg1_1 = do_frame(len(msg1_payload1) + 1, 1, msg1_payload1)
    msg2_payload1 = struct.pack('')
    msg2_1 = do_frame(len(msg2_payload1) + 1, 2, msg2_payload1)
    msg1_payload2 = struct.pack('>B', 2)
    msg1_2 = do_frame(len(msg1_payload2) + 1, 1, msg1_payload2)
    msg2_payload2 = struct.pack('>B', 0xff)
    msg2_2 = do_frame(len(msg2_payload2) + 1, 2, msg2_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1_1) + len(msg2_1) + len(msg1_2) + len(msg2_2), seq)
    commsdsl_write_packet(f, header + msg1_1 + msg2_1 + msg1_2 + msg2_2, time.time())

def pcap2(f):
    seq = 2000
    msg1_payload = struct.pack('>B', 1)
    msg1 = do_frame(len(msg1_payload) + 1, 1, msg1_payload)
    msg2_payload = struct.pack('')
    msg2 = do_frame(len(msg2_payload) + 1, 2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1) + len(msg2), seq)
    commsdsl_write_packet(f, header + msg1 + msg2, time.time())

    seq = 3000
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2) + len(msg2), seq)
    commsdsl_write_packet(f, header + msg2 + msg2, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)

if __name__ == '__main__':
    main()
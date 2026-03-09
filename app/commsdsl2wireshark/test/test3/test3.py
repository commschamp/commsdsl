import sys

from commsdsl_pcap_gen import *

def test3_frame(id, payload):
    prefix = struct.pack('!B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    msg1_payload = struct.pack('!I', 0xdeadbeef)
    msg1 = test3_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg1_payload_1 = struct.pack('!I', 0x11111111)
    msg1_1 = test3_frame(1, msg1_payload_1)
    msg1_payload_2 = struct.pack('!I', 0x22222222)
    msg1_2 = test3_frame(1, msg1_payload_2)
    raw_data = msg1_1 + msg1_2
    header = commsdsl_create_ethernet_ip_tcp_headers(len(raw_data), seq)
    commsdsl_write_packet(f, header + raw_data, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)

if __name__ == '__main__':
    main()
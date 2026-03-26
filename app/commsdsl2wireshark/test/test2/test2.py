import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('>B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    msg1_payload = struct.pack('>I', 0x11111111)
    msg1 = do_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('>II', 0x22222222, 0x33333333)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload = struct.pack('>IHI', 0x33333333, 0x4444, 0x55555555)
    msg3 = do_frame(3, msg3_payload)
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
import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('>B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    msg1_payload = struct.pack('>IHBBBfBBBB3sB2sBBIBI', 1, 2, 1, 0, 3, 1.23, 0xff, 4, 5, 3, b"bla", 2, b"\xaa\xbb", 2, 4, 1111, 4, 2222)
    msg1 = do_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('>Q', 0)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    msg2_payload = struct.pack('>Q', 0x7fffffff)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap4(f):
    seq = 4000
    msg2_payload = struct.pack('>Q', 0xffffffff)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap5(f):
    seq = 5000
    msg2_payload = struct.pack('>Q', 0x7fffffffffffffff)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap6(f):
    seq = 6000
    msg2_payload = struct.pack('>Q', 0xffffffffffffffff)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)
        pcap3(f)
        pcap4(f)
        pcap5(f)
        pcap6(f)

if __name__ == '__main__':
    main()
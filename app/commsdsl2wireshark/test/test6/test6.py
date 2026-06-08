import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('<B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    msg1_payload = struct.pack('<H', 12)
    msg1 = do_frame(1, msg1_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload = struct.pack('<BB', 128, 1)
    msg2 = do_frame(2, msg2_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload = struct.pack('<11B', 1, 5, 0x11, 2, 0, 1, 1, 3, 1, 1, 6)
    msg3 = do_frame(3, msg3_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3), seq)
    commsdsl_write_packet(f, header + msg3, time.time())

def pcap4(f):
    seq = 4000
    msg5_payload = struct.pack('<BBBB', 0x80, 0x7f, 0xfc, 0x00)
    msg5 = do_frame(5, msg5_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg5), seq)
    commsdsl_write_packet(f, header + msg5, time.time())

def pcap5(f):
    seq = 5000
    msg6_payload = struct.pack('<6B6B6B6B',
        0x80, 0x80, 0x80, 0x80, 0x80, 0x7f,
        0xfc, 0x80, 0x80, 0x80, 0x80, 0x00,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x7f,
        0xfc, 0x80, 0x80, 0x80, 0x80, 0x01)
    msg6 = do_frame(6, msg6_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg6), seq)
    commsdsl_write_packet(f, header + msg6, time.time())

def pcap6(f):
    seq = 6000
    msg7_payload = struct.pack('<2B2B3B2B',
        0x80, 0x01,
        0x80, 0x7f,
        0xff, 0xff, 0x03,
        0x80, 0x7c)
    msg7 = do_frame(7, msg7_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg7), seq)
    commsdsl_write_packet(f, header + msg7, time.time())

def pcap7(f):
    seq = 7000
    msg7_payload = struct.pack('<3B3B6B2B',
        0xff, 0xff, 0x7f,
        0xff, 0xff, 0x01,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x01,
        0x80, 0x70)
    msg7 = do_frame(7, msg7_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg7), seq)
    commsdsl_write_packet(f, header + msg7, time.time())

def pcap8(f):
    seq = 8000
    msg8_payload = struct.pack('<3B2B3B2B',
        0x81, 0xff, 0x7f,
        0xff, 0x00,
        0x83, 0xff, 0x7f,
        0xfc, 0x00)
    msg8 = do_frame(8, msg8_payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg8), seq)
    commsdsl_write_packet(f, header + msg8, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)
        pcap2(f)
        pcap3(f)
        pcap4(f)
        pcap5(f)
        pcap6(f)
        pcap7(f)
        pcap8(f)

if __name__ == '__main__':
    main()
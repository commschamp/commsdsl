import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('!B', id)
    return prefix + payload

def pcap1(f):
    seq = 1
    payload = struct.pack('>BBHH', 1, 3, 4, 5)
    msg1 = do_frame(1, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1), seq)
    commsdsl_write_packet(f, header + msg1, time.time())

def pcap2(f):
    seq = 2000
    payload = struct.pack('>HH', 1, 0xaabb)
    msg2 = do_frame(2, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2), seq)
    commsdsl_write_packet(f, header + msg2, time.time())

def pcap3(f):
    seq = 3000
    payload = struct.pack('>HBBBB', 0, 1, 0, 4, 6)
    msg3 = do_frame(3, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3), seq)
    commsdsl_write_packet(f, header + msg3, time.time())

def pcap4(f):
    seq = 4000
    payload = struct.pack('>BB', 1, 0)
    msg4 = do_frame(4, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg4), seq)
    commsdsl_write_packet(f, header + msg4, time.time())

def pcap5(f):
    seq = 5000
    payload = struct.pack('>HH', 1, 0xaabb)
    msg5 = do_frame(5, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg5), seq)
    commsdsl_write_packet(f, header + msg5, time.time())

def pcap6(f):
    seq = 6000
    payload = struct.pack('>BB', 1, 0)
    msg6 = do_frame(6, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg6), seq)
    commsdsl_write_packet(f, header + msg6, time.time())

def pcap7(f):
    seq = 7000
    payload1 = struct.pack('>BB', 0, 3)
    msg7_1 = do_frame(7, payload1)
    payload2 = struct.pack('>B', 1)
    msg7_2 = do_frame(7, payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg7_1) + len(msg7_2), seq)
    commsdsl_write_packet(f, header + msg7_1 + msg7_2, time.time())

def pcap8(f):
    seq = 8000
    payload1 = struct.pack('>BBBB', 1, 1, 1, 1)
    msg8_1 = do_frame(8, payload1)
    payload2 = struct.pack('>B', 0)
    msg8_2 = do_frame(8, payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg8_1) + len(msg8_2), seq)
    commsdsl_write_packet(f, header + msg8_1 + msg8_2, time.time())

def pcap9(f):
    seq = 9000
    payload = struct.pack('>BB', 1, 2)
    msg9 = do_frame(9, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg9), seq)
    commsdsl_write_packet(f, header + msg9, time.time())

def pcap10(f):
    seq = 10000
    payload = struct.pack('>BB3sB2HB3sBB', 0, 3, b"\x01\x02\x03", 2, 0x1111, 0x2222, 3, b"bla", 5, 6)
    msg10 = do_frame(10, payload)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg10), seq)
    commsdsl_write_packet(f, header + msg10, time.time())

def pcap11(f):
    seq = 11000
    payload1 = struct.pack('>BBBBBBB', 0xf, 1, 2, 3, 4, 5, 6)
    msg11_1 = do_frame(11, payload1)
    payload2 = struct.pack('>BBBBB', 0, 3, 4, 5, 6)
    msg11_2 = do_frame(11, payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg11_1) + len(msg11_2), seq)
    commsdsl_write_packet(f, header + msg11_1 + msg11_2, time.time())

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
        pcap9(f)
        pcap10(f)
        pcap11(f)

if __name__ == '__main__':
    main()
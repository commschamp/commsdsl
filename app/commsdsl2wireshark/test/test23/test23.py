import sys

from commsdsl_pcap_gen import *

def do_frame(id, version, payload):
    prefix = struct.pack('>BB', id, version)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg1_payload1 = struct.pack('>BB3s', 3, 0, b"bla")
    msg1_1 = do_frame(1, 5, msg1_payload1)
    msg1_payload2 = struct.pack('>BB5s', 5, 0, b"hello")
    msg1_2 = do_frame(1, 5, msg1_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg1_1) + len(msg1_2), seq)
    commsdsl_write_packet(f, header + msg1_1 + msg1_2, time.time())

def pcap2(f):
    seq = 2000
    msg2_payload1 = struct.pack('>BB', 3, 0)
    msg2_1 = do_frame(2, 1, msg2_payload1)
    msg2_payload2 = struct.pack('>BB5s', 5, 0, b"hello")
    msg2_2 = do_frame(2, 5, msg2_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg2_1) + len(msg2_2), seq)
    commsdsl_write_packet(f, header + msg2_1 + msg2_2, time.time())

def pcap3(f):
    seq = 3000
    msg3_payload1 = struct.pack('')
    msg3_1 = do_frame(3, 1, msg3_payload1)
    msg3_payload2 = struct.pack('>BB5s', 5, 0, b"hello")
    msg3_2 = do_frame(3, 5, msg3_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3_1) + len(msg3_2), seq)
    commsdsl_write_packet(f, header + msg3_1 + msg3_2, time.time())

def pcap4(f):
    seq = 4000
    msg4_payload1 = struct.pack('>BB2s', 2, 1, b"\x01\x02")
    msg4_1 = do_frame(4, 1, msg4_payload1)
    msg4_payload2 = struct.pack('>BB2sB1s', 0, 2, b"\x02\x03", 1, b"\x05")
    msg4_2 = do_frame(4, 5, msg4_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg4_1) + len(msg4_2), seq)
    commsdsl_write_packet(f, header + msg4_1 + msg4_2, time.time())

def pcap5(f):
    seq = 5000
    msg5_payload1 = struct.pack('>B3s', 3, b"bla")
    msg5_1 = do_frame(5, 1, msg5_payload1)
    msg5_payload2 = struct.pack('>BB3s', 0, 3, b"abc")
    msg5_2 = do_frame(5, 5, msg5_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg5_1) + len(msg5_2), seq)
    commsdsl_write_packet(f, header + msg5_1 + msg5_2, time.time())

def pcap6(f):
    seq = 6000
    msg6_payload1 = struct.pack('>B3s', 3, b"\x01\x02\x03")
    msg6_1 = do_frame(6, 1, msg6_payload1)
    msg6_payload2 = struct.pack('>BB3s', 0, 3, b"\x03\x02\x01")
    msg6_2 = do_frame(6, 5, msg6_payload2)
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg6_1) + len(msg6_2), seq)
    commsdsl_write_packet(f, header + msg6_1 + msg6_2, time.time())

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
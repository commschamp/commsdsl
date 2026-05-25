import sys

from commsdsl_pcap_gen import *

def do_frame(id, payload):
    prefix = struct.pack('>B', id)
    return prefix + payload

def pcap1(f):
    seq = 1000
    msg3_payload = struct.pack('')
    msg3 = do_frame(3, msg3_payload)
    msg4_payload = struct.pack('')
    msg4 = do_frame(4, msg4_payload) # Expected invalid id
    header = commsdsl_create_ethernet_ip_tcp_headers(len(msg3) + len(msg4), seq)
    commsdsl_write_packet(f, header + msg3 + msg4, time.time())

def main():
    with open(sys.argv[1], 'wb') as f:
        commsdsl_write_pcap_header(f)
        pcap1(f)

if __name__ == '__main__':
    main()
#include <cstdlib>
#include "Network.h"

Network::Network() { mode_ = Idle; }

Network::~Network() {}

void Network::reset() {
  remote_ = Socket::Address();
  seq_in_ = seq_out_ = 0;
  packets_out_cursor_ = 0;
  memset(packets_out_, 0, sizeof(packets_out_));
}

void Network::listen(int local_port) {
  L("NETWORK INFO: Will listening on *:%d", local_port);
  reset();
  socket_.bind(Socket::Address(local_port));
  mode_ = Server;
}

void Network::connect(const char *remote_host, int remote_port) {
  L("NETWORK INFO: Will connect to %s:%d", remote_host, remote_port);
  reset();
  remote_ = Socket::Address(remote_host, remote_port);
  mode_ = Client;
}

bool Network::write(const void *data, u32 size) {
  if (mode_ == Idle) return false;

  if (size > MAX_NET_PAYLOAD) {
    L("NETWORK FATAL: packet size %d is larger than max %d", size, MAX_NET_PAYLOAD);
    abort();
  }

  bool written = false;
  PacketInfo &pktinfo = packets_out_[packets_out_cursor_];
  if (pktinfo.size == 0) {
    pktinfo.size = size + 4;
    pktinfo.pkt.seq = seq_out_;
    memcpy(pktinfo.pkt.payload, data, size);
    seq_out_ = (seq_out_ + 1) & 0x7fffffffUL;
    packets_out_cursor_ = (packets_out_cursor_ + 1) % MAX_NET_PACKETS_QUEUE;
    written = true;
  } else L("NETWORK WARNING: Queue is full");

  // resend all other queued packet, including this new one
  /// \todo this will pollute network if write() is called too often
  send();

  return written;
}

void Network::send() {
  for (int i = 0; i < MAX_NET_PACKETS_QUEUE; ++i)
    if (packets_out_[i].size >= 4)
      socket_.send_to(remote_, &packets_out_[i].pkt, packets_out_[i].size);
}

const void *Network::read(u32 &size) {
  if (mode_ == Idle) return nullptr;

  Socket::Address from;
  for(;;) {
    packet_in_.size = socket_.recv_from(from, &packet_in_.pkt, sizeof(packet_in_.pkt));
    if (packet_in_.size == 0) break;
    if (packet_in_.size < 4) {
      L("NETWORK ERROR: packet %d is too small", packet_in_.size);
      continue;
    }

    if (from != remote_) {
      L("NETWORK WARNING: new remote addr %d.%d.%d.%d:%d",
        from.host>>24, (from.host>>16)&0xff, (from.host>>8)&0xff, from.host&0xff,
        from.port);
      remote_ = from;
    }

    u32 real_seq = packet_in_.pkt.seq & 0x7fffffffUL;
    if ((packet_in_.pkt.seq & 0x80000000UL) != 0)  {
      L("NETWORK INFO: seq %d ack recv", real_seq); 
      // this is an ACK packet
      // mark it as sent
      for (int i = 0; i < MAX_NET_PACKETS_QUEUE; ++i)
        if (packets_out_[i].pkt.seq == real_seq)
          packets_out_[i].size = 0;
    } else {
      // this is a real packet
      // check whether it is in order, ignore future packets
      /// \todo is it really important?
      if (real_seq > seq_in_) { /// \todo fix overflow
        L("NETWORK WARNING: Packet from future %d, local %d", real_seq, seq_in_);
        continue;
      }

      // send ack for past and current packets
      u32 ack = real_seq | 0x80000000UL;
      socket_.send_to(remote_, &ack, 4);

      // timing packets are the shit
      if (real_seq == seq_in_) {
        seq_in_ = (seq_in_ + 1) & 0x7fffffffUL;
        size = packet_in_.size - 4;
        return packet_in_.pkt.payload;
      }
    }
  }

  return nullptr;
}


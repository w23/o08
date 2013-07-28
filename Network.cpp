#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "Network.h"

Network::Network() {
  mode_ = Idle;
  reset();
  sock_ = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock_ < 0) {
    L("Failed to create socket");
    exit(-1);
  }
  fcntl(sock_, F_SETFL, O_NONBLOCK);
}

void Network::reset() {
  seq_in_ = seq_out_ = 0;
  packets_out_empty_ = MAX_NET_PACKETS_QUEUE;
}

void Network::listen(int local_port) {
  reset();
  sockaddr_in local_addr;
  memset(&local_addr, 0, sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  local_addr.sin_port = htons(local_port);
  if (0 > bind(sock_, (sockaddr*)&local_addr, sizeof(local_addr))) {
    L("Failed to bind socket");
    exit(-1);
  }
  mode_ = Server;
}

void Network::connect(const char *remote_host, int remote_port) {
  reset();
  memset(&remote_addr_, 0, sizeof(remote_addr_));
  remote_addr_.sin_family = AF_INET;
  remote_addr_.sin_addr.s_addr = inet_addr(remote_host);
  remote_addr_.sin_port = htons(remote_port);
  mode_ = Client;
}

Network::~Network() {
  close(sock_);
}

u32 Network::receive(void *payload) {
  if (mode_ == Idle) return 0;
  RawPacket packet;
  sockaddr_in from;
  for(;;) {
    socklen_t addrlen = sizeof(from);
    ssize_t size = recvfrom(sock_, &packet, sizeof(RawPacket), 0,
      (sockaddr*)&from, &addrlen);
    if (size == -1) {
      /// \todo if (errno != EWOULDBLOCK)
      break;
    }

    if (size < sizeof(RawPacket::Header)) {
      L("Bad packet size %d, should be at least %d", size, sizeof(RawPacket::Header));
      continue;
    }

    // if incoming packet is an ack packet
    if (packet.header.code == RawPacket::Ack) {
      // find corresponding outbound packet
      int i;
      for (i = 0; i < MAX_NET_PACKETS_QUEUE; ++i)
        if (packets_out_[i].pkt.header.seq == packet.header.seq) {
          // if found -- mark as acknowledged and stop
          packets_out_[i].pkt.header.code = 0;
          ++packets_out_empty_;
          break;
        }
      // if no corresponding packet was found, become surprised
      if (i == MAX_NET_PACKETS_QUEUE)
        L("WARNING: RawPacket::Ack for unknown packet seq %d (current outbound seq = %d)",
          packet.header.seq, seq_out_);

      // anyway, this was a technical packet, continue to a next one
      continue;
    }

    // if incoming packet is out of order, ignore it for simpliticy -- it will get resend
    /// \todo actually build a queue of incoming packets to consume in right order
    if (packet.header.seq > seq_in_) {
      L("WARNING: Out of order packet with seq %d (current: %d)",
        packet.header.seq, seq_in_);
      continue;
    }

    // notify sender that we've received this
    RawPacket::Header ack_pkt = {RawPacket::Ack, packet.header.seq};
    ssize_t sent = sendto(sock_, &ack_pkt, sizeof(ack_pkt), 0,
      (sockaddr*)&from, sizeof(from));
    if (sent != sizeof(ack_pkt)) {
      L("ERROR: Could not send ACK packet for seq %d", packet.header.seq);
      continue;
    }

    // if this packet is the one we expect, process
    if (packet.header.seq == seq_in_) {
      ++seq_in_; // update sequence number
      memcpy(payload, packet.payload, size);
      return packet.header.code;
    }
  }
  return 0;
}

void Network::send() {
  packets_out_empty_ = 0;
  for (int i = 0; i < MAX_NET_PACKETS_QUEUE; ++i)
    if (packets_out_[i].pkt.header.code != RawPacket::Ack) {
      ssize_t sent = sendto(sock_, &packets_out_[i].pkt, packets_out_[i].size,
        0, (sockaddr*)&remote_addr_, sizeof(remote_addr_));
      if (sent != packets_out_[i].size)
        L("ERROR: Could not send a packet %d with size %d", packets_out_[i].pkt.header.seq, packets_out_[i].size);
    } else ++packets_out_empty_;
}

void Network::enqueue(const void *payload, u32 size, u32 code) {
  for (int i = 0; i < MAX_NET_PACKETS_QUEUE; ++i)
    if (packets_out_[i].pkt.header.code == RawPacket::Ack) {
      packets_out_[i].size = size;
      packets_out_[i].pkt.header.code = code;
      memcpy(packets_out_[i].pkt.payload, payload, size);
      return;
    }
  L("FATAL: Network ueue is full!");
}

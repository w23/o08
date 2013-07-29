#pragma once
#include "Socket.h"

class Network {
public:
  Network();
  ~Network();

  void listen(int local_port); // i am a server
  void connect(const char *remote_host, int remote_port); // NO U

  bool write(const void *data, u32 size);
  const void *read(u32 &size);

  void send();
private:
  void reset();
  enum Mode {
    Idle, Server, Client
  } mode_;
  Socket socket_;

  Socket::Address remote_;
  u32 seq_in_, seq_out_;
  struct Packet {
    u32 seq;
    u8 payload[MAX_NET_PAYLOAD];
  };
  struct PacketInfo {
    u32 size;
    Packet pkt;
  } packets_out_[MAX_NET_PACKETS_QUEUE];
  u32 packets_out_cursor_;
  PacketInfo packet_in_;
};

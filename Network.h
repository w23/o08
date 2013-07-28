#pragma once
#include <netinet/in.h>
#include "config.h"
#include <kapusha/core.h>

using namespace kapusha;

class Network {
public:
  Network();
  ~Network();

  void listen(int local_port); // i am a server
  void connect(const char *remote_host, int remote_port); // NO U

  /// \warning should be called before send call in one update iteration
  /// \param payload should point to at least MAX_NET_PAYLOAD bytes
  /// \returns packet code, or 0 if no packet
  u32 receive(void *payload);

  /// \warning should be called between receive() and send() calls
  /// \returns count of available output slots
  void enqueue(const void *payload, u32 size, u32 code);
  u32 enqueue_get_free() const { return packets_out_empty_; }

  /// send all pending and not acknowledged packets
  /// \warning should be called after receive calls in one update iteration
  void send();

private:
  void reset();
  int sock_;
  enum Mode {
    Idle, Server, Client
  } mode_;

  sockaddr_in remote_addr_;
  u32 seq_in_, seq_out_;
  struct RawPacket {
    enum Code {
      Ack = 0,
      _UserCodeSpaceBegin
    };
    struct Header {
      u32 code;
      u32 seq;
    } header;
    u8 payload[MAX_NET_PAYLOAD];
    RawPacket() { header.code = Ack; }
  };
  struct PacketInfo {
    u32 size;
    RawPacket pkt;
  } packets_out_[MAX_NET_PACKETS_QUEUE];
  u32 packets_out_empty_;
};

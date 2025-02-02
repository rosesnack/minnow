#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
using namespace std;

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), sn_( isn ), initial_RTO_ms_( initial_RTO_ms ), RTO_(initial_RTO_ms), pq(priority_queue<TCPSenderMessage, vector<TCPSenderMessage>, CompareSeqno>{})
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  bool SYN_ = false;
  bool FIN_ = false;
  Wrap32 isn_;
  Wrap32 sn_;
  uint64_t initial_RTO_ms_;
  uint64_t RTO_;
  uint64_t consecutive_retransmissions_ = 0;
  uint16_t window_size_ = 1;
  struct CompareSeqno {
    bool operator()(const TCPSenderMessage& lhs, const TCPSenderMessage& rhs) const {
        return rhs.seqno < lhs.seqno;
    }
  };
  priority_queue<TCPSenderMessage, vector<TCPSenderMessage>, CompareSeqno> pq;
  bool timer_ = false; //是否启动计时器
  uint64_t time_passed_ = 0;
  uint64_t sequence_numbers_in_flight_ = 0;
};

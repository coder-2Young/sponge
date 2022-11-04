#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _bytes_in_flight(0)
    , _window_size(1) 
    , RTO(INIT_RTO) {}

uint64_t TCPSender::bytes_in_flight() const { return {_bytes_in_flight}; }

void TCPSender::fill_window() {
    TCPSegment seg;
    if(next_seqno_absolute() == 0)
    {
        seg.header().syn = true;
        seg.header().seqno = _isn;
        _segments_out.push(seg);
        _outstanding_seg.push(seg);
        _next_seqno+=seg.length_in_sequence_space();
        _bytes_in_flight+=seg.length_in_sequence_space(); 
        return;
    }
    else if(_next_seqno > _bytes_in_flight && !_stream.eof())
    {
        if(_stream.buffer_empty())
            return;
        else
        {
            seg.header().seqno = wrap(_next_seqno,_isn);
            Buffer buf{_stream.read(_window_size<TCPConfig::MAX_PAYLOAD_SIZE ? _window_size:TCPConfig::MAX_PAYLOAD_SIZE)};
            seg.payload() = buf;
            _segments_out.push(seg);
            _outstanding_seg.push(seg);
            _next_seqno+=seg.length_in_sequence_space();
            _bytes_in_flight+=seg.length_in_sequence_space(); 
            return;
        }
    }
    else if (_stream.eof() && _next_seqno < _stream.bytes_written()+2)
    {
        seg.header().seqno = wrap(_next_seqno,_isn); 
        seg.header().fin = true;
        _segments_out.push(seg);
        _outstanding_seg.push(seg);
        _next_seqno+=seg.length_in_sequence_space();
        _bytes_in_flight+=seg.length_in_sequence_space(); 
        return;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
     if(ackno == wrap(_next_seqno, _isn))
     {
        _bytes_in_flight -= _outstanding_seg.front().length_in_sequence_space();
        _window_size = window_size;
        _outstanding_seg.pop();
     }
     if(window_size) DUMMY_CODE();
     
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

unsigned int TCPSender::consecutive_retransmissions() const { return {}; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    _segments_out.push(seg);
}

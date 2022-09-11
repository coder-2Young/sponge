#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if(seg.header().syn)
    {
        _isn = seg.header().seqno;
        _ackno_has_value = true;
        _ackno = _isn;
    }
    if(_ackno_has_value)
    {
        string data = seg.payload().copy();
        uint64_t index = unwrap(seg.header().seqno,_isn,_check_point);
        if(seg.header().syn) index += 1;
        bool eof = seg.header().fin;
        uint64_t prev_buffer_size = stream_out().buffer_size();
        _reassembler.push_substring(data, index-1, eof);
        uint64_t past_buffer_size = stream_out().buffer_size();
        _ackno = _ackno + past_buffer_size - prev_buffer_size;
        if(seg.header().syn) _ackno = _ackno + 1; 
        if(stream_out().input_ended()) _ackno = _ackno + 1;
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(!_ackno_has_value)
        return {};
    else
    {
        return _ackno;
    } 
}

size_t TCPReceiver::window_size() const { return {_capacity - stream_out().buffer_size()}; }

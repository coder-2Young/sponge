#include "byte_stream.hh"

#include <iostream>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    :buffer(""), _size(capacity),_bytes_written(0), _bytes_read(0),_end(false){
}

size_t ByteStream::write(const string &data) {
    if(input_ended()) return 0;
    size_t len = min(data.length(), remaining_capacity());
    buffer += data.substr(0,len);
    _bytes_written += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t true_len = min(len, buffer_size());
    return {buffer.substr(0,true_len)};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    if(len >= buffer_size())
    {
        buffer = "";
        _bytes_read += buffer_size();
        return;
    }
    else
    {
        buffer = buffer.substr(len,buffer_size() - len);
        _bytes_read += len;  
        return;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string out_data = peek_output(len);
    pop_output(len);
    return {out_data};
}
void ByteStream::end_input() {
    _end = true;
}

bool ByteStream::input_ended() const { return {_end}; }

size_t ByteStream::buffer_size() const {
    size_t size = _bytes_written - _bytes_read;
    return size;
}

bool ByteStream::buffer_empty() const { 
    return {buffer_size() == 0}; 
}

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return {_bytes_written}; }

size_t ByteStream::bytes_read() const { return {_bytes_read}; }

size_t ByteStream::remaining_capacity() const { return {_size - buffer_size()}; }


#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
    : _output(capacity), _capacity(capacity),_idx_to_str(), _input_idx(), _current_index(0), _eof_index(-1), _input_size(0) {}

void StreamReassembler::write_in_input(const string &data, const size_t index)
{
    _input_idx.insert(index);
    _idx_to_str[index] = data;
    _input_size += data.size();
}

void StreamReassembler::merge_input(const string &data, const size_t index)
{
    size_t write_tail_index = index + data.size();
    string write_data = data;
    size_t write_index = index;
    
    set<size_t> remove_set;
    set<size_t>::iterator it = _input_idx.begin();
    while(it!=_input_idx.end())
    {
        size_t input_index = *it;
        string input_data = _idx_to_str[input_index];
        size_t input_tail_index = input_index+input_data.size();

        if(input_tail_index < write_index) // 不相交(左)
            ++it;
        else if(input_index > write_tail_index) // 不相交（右）
            break;
        else if (input_index <= write_index && input_tail_index >= write_tail_index)// 被包含
            return;
        else if (input_index >= write_index && input_tail_index <= write_tail_index)// 包含
        {
            remove_from_input(input_data,input_index);
            it = _input_idx.begin();
        }

        else if(input_index <= write_tail_index && input_tail_index >= write_tail_index)
        {// 在右半边
            write_data += input_data.substr(write_tail_index - input_index);
            write_tail_index = input_tail_index;
            remove_from_input(input_data,input_index); 
            it = _input_idx.begin();
        }
        else if(input_index < write_index && input_tail_index >= write_index)
        {// 在左半边
            write_data = input_data.substr(0, write_index-input_index) + write_data;
            write_index = input_index;
            remove_from_input(input_data,input_index);
            it = _input_idx.begin();
        }

        else
            break;
    }
    write_in_input(write_data,write_index);
}   

void StreamReassembler::remove_from_input(const string &data, const size_t index)
{
    _input_idx.erase(index);
    _idx_to_str.erase(index);
    _input_size -= data.size();
}

bool StreamReassembler::write_in_output(const string &data, const size_t size)
{
    size_t remaining_capacity = _capacity - _output.buffer_size();
    _output.write(data);
    _current_index += min(size,remaining_capacity);
    return size <= remaining_capacity;// 如果没被cut，返回true
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {

    merge_input(data,index);
    if(data.empty() && eof) 
    {
        _output.end_input();
        return;
    }
    if(eof) _eof_index = index + data.size(); // 记录eof_index

    if(!_input_idx.empty())
    {
        size_t input_index = *_input_idx.begin();
        string input_data = _idx_to_str[input_index];
        if(input_index <= _current_index)
        {
            if(input_index + input_data.size() <= _current_index)
            {
                remove_from_input(input_data,input_index);
                return;
            }
            else
            {
                size_t buffer_capacity = _capacity - _output.buffer_size();
                remove_from_input(input_data,input_index);
                input_data = input_data.substr(_current_index - input_index);
                input_index = _current_index;

                size_t write_size = min(input_data.size(), buffer_capacity);
                write_in_output(input_data.substr(0,write_size), write_size);

                if(input_index + write_size  == _eof_index) _output.end_input();
            }
        }
    }

    while(_output.buffer_size() + _input_size > _capacity)
    {
        size_t out_size = _input_size-(_capacity - _output.buffer_size());
        set<size_t>::iterator it = _input_idx.end();
        --it;
        size_t input_index = *it;
        string input_data = _idx_to_str[input_index];
        if(out_size >= input_data.size())
        {
            remove_from_input(input_data,input_index);
        }
        else
        {
            input_data = input_data.substr(0, input_data.size()-out_size);
            _input_size -= out_size;
        }
    }
}

size_t StreamReassembler::unassembled_bytes() const { return {_input_size};}

bool StreamReassembler::empty() const { return {_input_idx.empty()&&_output.buffer_empty()}; }

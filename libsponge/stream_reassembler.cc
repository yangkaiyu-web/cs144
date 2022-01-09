#include "stream_reassembler.hh"
#include <iostream>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(this->_capacity <= 0)
        return;

    size_t curr_maxindex = min(this->_capacity - (this->_output).buffer_size() + 
                            this->unassembled_index, index + data.size()) - 1;

    //cout << "data:" << data << "index:" << index << "eof:" << eof << endl;
    if(eof)
    {
        this->stream_eof = true;
        this->Endindex = index + data.size() - 1;
        //cout<< "Endindex:" <<this->Endindex <<" unassembled_index:"<<this->unassembled_index<<endl;
        if(this->Endindex < this->unassembled_index)
            this->_output.end_input();
    }

    size_t i = 0;
    for(auto c : data)
    {
        if(index + i > curr_maxindex)
            break;
        if(this->m.count(index + i) == 0)
        {
            this->m[index + i] = c;
            ++this->unassembled_cnt;
        }
        ++i;
    }

    auto iter = m.find(this->unassembled_index);
    if(iter != m.end())
    {
        int newAssembled_cnt = 0;
        string str;
        for(; iter != m.end(); ++iter)        
        {
            if(iter->first != this->unassembled_index + newAssembled_cnt || 
                (this->stream_eof && iter->first > this->Endindex))
            {
                ++iter;
                break;
            }
            str += iter->second;
            newAssembled_cnt += 1;
        }
        this->_output.write(str);
        this->unassembled_cnt -= newAssembled_cnt;
        this->unassembled_index += newAssembled_cnt;
        if(this->stream_eof && (--iter)->first >= this->Endindex)
            this->_output.end_input();
    }
    
}

size_t StreamReassembler::unassembled_bytes() const { return this->unassembled_cnt; }

bool StreamReassembler::empty() const { return this->unassembled_cnt == 0; }

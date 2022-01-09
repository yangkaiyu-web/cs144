#include "tcp_receiver.hh"
#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if(seg.header().syn == true)
    {
        this->SYN = true;
        this->FIN = seg.header().fin;
        this->ISN = seg.header().seqno;
    }
    if(this->SYN)
    {
        this->_reassembler.push_substring(seg.payload().copy(), 
                    unwrap(seg.header().seqno - (seg.header().syn ? 0 : 1), this->ISN, this->unassembled_bytes()),
                    seg.header().fin);
        //cout << "data:" << seg.payload().copy() << endl;
        //cout << "seqno:" << seg.header().seqno << endl;
    }
    if(seg.header().fin == true)
    {
        this->FIN = true;
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(this->SYN)
    {
        return this->ISN + this->_reassembler.unassembledIndex() + 
                1 + (this->FIN && (this->unassembled_bytes() == 0) ? 1 : 0);
    }
    else
        return std::nullopt;
}

size_t TCPReceiver::window_size() const {
    return this->_capacity - this->stream_out().buffer_size();
}

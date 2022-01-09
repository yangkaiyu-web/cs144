#include "wrapping_integers.hh"
#include <iostream>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint32_t n32 = n & 0xFFFFFFFF;
    return isn + n32;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    int64_t t = n - isn;
    uint64_t t1 = t + ((checkpoint & (0xFFFFFFFF00000000)) - (1ul << 32));
    uint64_t t2 = t + (checkpoint & (0xFFFFFFFF00000000));
    uint64_t t3 = t + ((checkpoint & (0xFFFFFFFF00000000)) + (1ul << 32));
    uint64_t a1 = checkpoint > t1 ? (checkpoint - t1) : (t1 - checkpoint);
    uint64_t a2 = checkpoint > t2 ? (checkpoint - t2) : (t2 - checkpoint);
    uint64_t a3 = checkpoint > t3 ? (checkpoint - t3) : (t3 - checkpoint);

    if(a2 > a1 && a3 > a1)
        return t1;
    else if(a1 > a2 && a3 > a2)
        return t2;
    else
        return t3;
}

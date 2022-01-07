#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t pcapacity): capacity(pcapacity) {}

size_t ByteStream::write(const string &data) {
    int write_cnt = (this->capacity - this->dq.size()) > data.length() ? data.length() :
        (this->capacity - this->dq.size());
    this->dq.insert(dq.end(), data.cbegin(), data.cbegin() + write_cnt);
    this->byte_wcnt += write_cnt;
    return write_cnt;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    if((this->dq.cend()-this->dq.cbegin()) > static_cast<long int>(len))
        return string(this->dq.cbegin(),this->dq.cbegin() + len);
    else
        return string(this->dq.cbegin(),this->dq.cend());
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    int pop_size = this->dq.size() > len ? len : this->dq.size();
    this->dq.erase(this->dq.begin(),this->dq.begin() + pop_size);
    this->byte_rcnt += pop_size;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string s = peek_output(len);
    pop_output(len);
    return s;
}

void ByteStream::end_input() { 
    this->endWrite_flag = true;
}

bool ByteStream::input_ended() const { return this->endWrite_flag; }

size_t ByteStream::buffer_size() const { return this->dq.size(); }

bool ByteStream::buffer_empty() const { return this->dq.size() == 0; }

bool ByteStream::eof() const { return this->endWrite_flag && 
                            this->byte_wcnt == this->byte_rcnt;}

size_t ByteStream::bytes_written() const { return this->byte_wcnt; }

size_t ByteStream::bytes_read() const { return this->byte_rcnt; }

size_t ByteStream::remaining_capacity() const { return this->capacity - this->dq.size(); }

#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <string>
#include <iostream>

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
    , mostRecent_ackno(_isn)
    , _initial_retransmission_timeout{retx_timeout}
    , RTO{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { 
    //cout<< "next_seq:"<<this->next_seqno()<<" mostrecent_ackno:"<<this->mostRecent_ackno<<endl;
    return this->next_seqno() - this->mostRecent_ackno; 
    }

void TCPSender::fill_window() {
    do
    {
        if(this->closed())
            return;

        TCPSegment segm{};
        segm.header().seqno = this->next_seqno();
        segm.header().syn = (this->next_seqno() == this->_isn);
        uint16_t seq_len = 0;

        if(segm.header().syn)
            seq_len = this->recv_window - 1;
        else if(this->recv_window >= this->bytes_in_flight())
            seq_len = this->recv_window - this->bytes_in_flight();
        else
            return;

        if(this->recv_window == 0)
            seq_len += 1;
        
        const string str(this->stream_in().read(seq_len > TCPConfig::MAX_PAYLOAD_SIZE ? 
                    TCPConfig::MAX_PAYLOAD_SIZE : seq_len));
        segm.payload() = string(str);

        if(this->stream_in().eof() && segm.length_in_sequence_space() < seq_len)
        {
            segm.header().fin = true;
            this->_closed = true;
        }

        if(segm.length_in_sequence_space() != 0)
        {
            this->clock_running = true;
            this->_next_seqno += segm.length_in_sequence_space();
            this->segments_out().push(segm);
            this->unack_segments_out().push(segm);
        }
        else
            break;
        //cout<<"readlen:"<<read_len<<endl;
    } while (true);
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{
    int32_t t;
    if(this->next_seqno() - ackno >= 0 && (t = (ackno - this->mostRecent_ackno)) >= 0)
    {
        this->recv_window = window_size;
        std::queue<TCPSegment>& s = this->unack_segments_out();
        if(ackno - (s.front().header().seqno + s.front().length_in_sequence_space()) >= 0)
        {
            while (true)
            {
                if(s.empty())
                {
                    this->clock_running = false;
                    break;
                }

                if(ackno - (s.front().header().seqno + s.front().length_in_sequence_space()) >= 0)
                    s.pop();
                else
                    break;
            }
            this->consecutive_retransmissions_cnt = 0;
            this->mostRecent_ackno = ackno;
            this->RTO = this->_initial_retransmission_timeout;
            fill_window();
            this->retransmission_timer = 0;
        }
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{
    if(clock_running)
        this->retransmission_timer += ms_since_last_tick;
    cout<< "clock_running:"<<clock_running<<"retransmission_timer:"<<this->retransmission_timer
        << "RTO:"<<this->RTO<<endl;

    if(this->retransmission_timer >= this->RTO)
    {
        this->segments_out().push(this->unack_segments_out().front());
        
        if(this->recv_window > 0)
        {
            ++this->consecutive_retransmissions_cnt;
            this->RTO *= 2;
        }
        this->retransmission_timer = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return this->consecutive_retransmissions_cnt; }

void TCPSender::send_empty_segment() 
{
    TCPSegment segm;
    segm.header().seqno = this->next_seqno();

    segm.payload() = string({});
    this->segments_out().push(segm);
}

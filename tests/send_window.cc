#include "sender_harness.hh"
#include "wrapping_integers.hh"

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

using namespace std;

int main() {
    try {
        auto rd = get_random_generator();

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;
            cout<<1<<endl;

            TCPSenderTestHarness test{"Initial receiver advertised window is respected", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(4));
            test.execute(ExpectNoSegment{});
            test.execute(WriteBytes{"abcdefg"});
            test.execute(ExpectSegment{}.with_no_flags().with_data("abcd"));
            test.execute(ExpectNoSegment{});
        }

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;
            cout<<2<<endl;

            TCPSenderTestHarness test{"Immediate window is respected", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(6));
            test.execute(ExpectNoSegment{});
            test.execute(WriteBytes{"abcdefg"});
            test.execute(ExpectSegment{}.with_no_flags().with_data("abcdef"));
            test.execute(ExpectNoSegment{});
        }

        {
            const size_t MIN_WIN = 5;
            const size_t MAX_WIN = 100;
            const size_t N_REPS = 1000;
            cout<<3<<endl;
            for (size_t i = 0; i < N_REPS; ++i) {
                size_t len = MIN_WIN + rd() % (MAX_WIN - MIN_WIN);
                TCPConfig cfg;
                WrappingInt32 isn(rd());
                cfg.fixed_isn = isn;

                TCPSenderTestHarness test{"Window " + to_string(i), cfg};
                test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
                test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(len));
                test.execute(ExpectNoSegment{});
                test.execute(WriteBytes{string(2 * N_REPS, 'a')});
                test.execute(ExpectSegment{}.with_no_flags().with_payload_size(len));
                test.execute(ExpectNoSegment{});
            }
        }

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;
            cout<<4<<endl;

            TCPSenderTestHarness test{"Window growth is exploited", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(4));
            test.execute(ExpectNoSegment{});
            test.execute(WriteBytes{"0123456789"});
            test.execute(ExpectSegment{}.with_no_flags().with_data("0123"));
            test.execute(AckReceived{WrappingInt32{isn + 5}}.with_win(5));
            test.execute(ExpectSegment{}.with_no_flags().with_data("45678"));
            test.execute(ExpectNoSegment{});
        }

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;
            cout<<5<<endl;

            TCPSenderTestHarness test{"FIN flag occupies space in window", cfg};
            cout<<5.1<<endl;
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            cout<<5.2<<endl;
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(7));
            cout<<5.3<<endl;
            test.execute(ExpectNoSegment{});
            cout<<5.4<<endl;
            test.execute(WriteBytes{"1234567"});
            cout<<5.5<<endl;
            test.execute(Close{});
            cout<<5.6<<endl;
            test.execute(ExpectSegment{}.with_no_flags().with_data("1234567"));
            cout<<5.7<<endl;
            test.execute(ExpectNoSegment{});  // window is full
            cout<<5.8<<endl;
            test.execute(AckReceived{WrappingInt32{isn + 8}}.with_win(1));
            cout<<5.9<<endl;
            test.execute(ExpectSegment{}.with_fin(true).with_data(""));
            cout<<5.10<<endl;
            test.execute(ExpectNoSegment{});
        }

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;
            cout<<6<<endl;

            TCPSenderTestHarness test{"FIN flag occupies space in window (part II)", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(7));
            test.execute(ExpectNoSegment{});
            test.execute(WriteBytes{"1234567"});
            test.execute(Close{});
            test.execute(ExpectSegment{}.with_no_flags().with_data("1234567"));
            test.execute(ExpectNoSegment{});  // window is full
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(8));
            test.execute(ExpectSegment{}.with_fin(true).with_data(""));
            test.execute(ExpectNoSegment{});
        }

        {
            TCPConfig cfg;
            WrappingInt32 isn(rd());
            cfg.fixed_isn = isn;
            cout<<7<<endl;
            TCPSenderTestHarness test{"Piggyback FIN in segment when space is available", cfg};
            test.execute(ExpectSegment{}.with_no_flags().with_syn(true).with_payload_size(0).with_seqno(isn));
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(3));
            test.execute(ExpectNoSegment{});
            test.execute(WriteBytes{"1234567"});
            test.execute(Close{});
            test.execute(ExpectSegment{}.with_no_flags().with_data("123"));
            test.execute(ExpectNoSegment{});  // window is full
            test.execute(AckReceived{WrappingInt32{isn + 1}}.with_win(8));
            test.execute(ExpectSegment{}.with_fin(true).with_data("4567"));
            test.execute(ExpectNoSegment{});
        }
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    return EXIT_SUCCESS;
}

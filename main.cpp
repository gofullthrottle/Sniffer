#include <iostream>
#include <set>
#include <string>
#include "tins/tins.h"
#include <signal.h>

using namespace Tins;
using namespace std;

bool print0 = false;
void println(string str) {
    if(str.size() > 0) {
        cout << str;
        if(print0) {
            cout << '\0';
        } else {
            cout << endl;
        }
        cout.flush();
    }
}

string lower_string(const string& str) {
    string lower;
    transform(str.begin(), str.end(), back_inserter(lower), ::tolower);
    return lower;
}

string::size_type ifind(const string& str, const string& substr) {
    return lower_string(str).find(lower_string(substr));
}

string parse_get(Packet packet) {
    stringstream ss;
    try {
        // first check that we have everything we need
        const RawPDU &raw = packet.pdu()->rfind_pdu<RawPDU>();

        // convert the raw data to a printable string
        stringstream data, data_printable;
        int n = raw.payload().size();
        for(int i = 0; i < n; i++){
            char ch = raw.payload()[i];
            data << ch;
            if(isprint(ch)) {
                if(ch == '"') {
                    data_printable << "\"";
                }
                data_printable << ch;
            } else {
                data_printable << ".";
            }
        }

        // search for GET and Host: in the data
        string data_str = data.str();
        int get_index = data_str.find("GET ", 0);
        int host_index = data_str.find("Host: ", 0);

        if(get_index == -1 || host_index == -1) {
            // cout << data_printable.str() << endl;
            throw runtime_error("Raw TCP data does not contain a GET request.");
        }

        int jpg_index = ifind(data_str, "jpg");
        int jpeg_index = ifind(data_str, "jpeg");
        if(jpg_index == -1 && jpeg_index == -1) {
            throw runtime_error("Not a JPG.");
        }
        
        int end = data_str.find(" ", get_index+4);
        string get = data_str.substr(get_index+4,end-(get_index+4));
        end = data_str.find("\r", host_index+6);
        string host = data_str.substr(host_index+6,end-(host_index+6));

        ss << "http://" << host << get;
    } catch (...) {
    }
    return ss.str();
}

int main(int argc, char* argv[]) {

    string interface = "en0";
    if(argc > 1){
        interface = argv[1];
    }

    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_rfmon(true);
    config.set_filter("type data and tcp port http");

    Sniffer sniffer(interface, config);

    while(true) {
        try {
            Packet packet = sniffer.next_packet();
            if(packet) {
                println(parse_get(packet));
            }
        } catch (...) {
        }
    }
}

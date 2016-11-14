#include <iostream>
#include <set>
#include <string>
#include "tins/tins.h"
#include <signal.h>

using namespace Tins;
using namespace std;

// string lower_string(const string& str) {
//     string lower;
//     transform(str.begin(), str.end(), back_inserter(lower), ::tolower);
//     return lower;
// }

// string::size_type ifind(const string& str, const string& substr) {
//     return lower_string(str).find(lower_string(substr));
// }

template <class T>
HWAddress<6> get_src_addr(const T& data) {
    if(!data.from_ds() && !data.to_ds())
        return data.addr2();
    if(!data.from_ds() && data.to_ds())
        return data.addr2();
    return data.addr3();
}

template <class T>
HWAddress<6> get_dst_addr(const T& data) {
    if(!data.from_ds() && !data.to_ds())
        return data.addr1();
    if(!data.from_ds() && data.to_ds())
        return data.addr3();
    return data.addr1();
}

string parse_types(const Packet& packet) {
    stringstream ss;
    try {
        const PDU *cur = packet.pdu();
        while(cur) {
            ss << Utils::to_string(cur->pdu_type()) << " ";
            cur = cur->inner_pdu();
        }
        ss << "(" << packet.pdu()->size() << " bytes)";
    } catch (...) {
    }
    return ss.str();
}

string parse_radiotap(const Packet& packet, const RadioTap &rt) {
    stringstream ss;

    try {
        unsigned int timestamp = packet.timestamp().seconds();
        ss << "\"timestamp\":" << timestamp << ",";
    } catch (...) {
    }

    try {
        int dbm_signal = rt.dbm_signal();
        ss << "\"dbm_signal\":" << dbm_signal << ",";
    } catch (...) {
    }

    try {
        int channel_freq = rt.channel_freq();
        ss << "\"channel_freq\":" << channel_freq << ",";
    } catch (...) {
    }

    return ss.str();
}

string parse_beacon(Packet packet) {
    stringstream ss;
    try {
        const RadioTap &rt =  packet.pdu()->rfind_pdu<RadioTap>();
        const Dot11Beacon &data = packet.pdu()->rfind_pdu<Dot11Beacon>();
        string ssid = data.ssid();
        HWAddress<6> src_addr = get_src_addr(data);
        bool privacy = data.capabilities().privacy();
        ss << "{" <<
            "\"type\":\"beacon\"," <<
            parse_radiotap(packet, rt) <<
            "\"src_addr\":\"" << src_addr << "\"," <<
            "\"ssid\":\"" << ssid << "\"," <<
            "\"privacy\":\"" << (privacy ? "true" : "false") << "\"" <<
            "}";
    } catch (...) {
    }
    return ss.str();
}

string parse_probe(Packet packet) {
    stringstream ss;
    try {
        // first check that it's a probe request frame
        packet.pdu()->rfind_pdu<Tins::Dot11ProbeRequest>();
        const RadioTap &rt =  packet.pdu()->rfind_pdu<RadioTap>();
        const Dot11ManagementFrame &data = packet.pdu()->rfind_pdu<Dot11ManagementFrame>();
        string ssid = data.ssid();
        HWAddress<6> src_addr = get_src_addr(data);
        ss << "{" <<
            "\"type\":\"probe\"," <<
            parse_radiotap(packet, rt) <<
            "\"src_addr\":\"" << src_addr << "\"," <<
            "\"ssid\":\"" << ssid << "\"" <<
            "}";
    } catch (...) {
    }
    return ss.str();
}

string parse_ip(Packet packet) {
    stringstream ss;
    try {
        // first check that we have everything we need
        const RadioTap &rt =  packet.pdu()->rfind_pdu<RadioTap>();
        const IP &ip = packet.pdu()->rfind_pdu<IP>();

        string src_ip = ip.src_addr().to_string();
        string dst_ip = ip.dst_addr().to_string();

        ss << "{" <<
            "\"type\":\"ip\"," <<
            parse_radiotap(packet, rt) <<
            "\"src\":{" <<
                "\"ip\":\"" << src_ip << "\"" <<
                "}," <<
            "\"dst\":{" <<
                "\"ip\":\"" << dst_ip << "\"" <<
                "}" <<
            "}";
    } catch (...) {
    }
    return ss.str();
}

string parse_get(Packet packet) {
    stringstream ss;
    try {
        // first check that we have everything we need
        const RadioTap &rt =  packet.pdu()->rfind_pdu<RadioTap>();
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

        // int jpg_index = data_str.find("jpg", 0);
        // int jpeg_index = data_str.find("jpeg", 0);
        // if(jpg_index == -1 && jpeg_index == -1) {
        //     throw runtime_error("Not a JPG.");
        // }
        
        int end = data_str.find(" ", get_index+4);
        string get = data_str.substr(get_index+4,end-(get_index+4));
        end = data_str.find("\r", host_index+6);
        string host = data_str.substr(host_index+6,end-(host_index+6));

        // ss << host << get;
        // return ss.str();
        
        const IP &ip = packet.pdu()->rfind_pdu<IP>();
        const TCP &tcp = packet.pdu()->rfind_pdu<TCP>();

        string src_ip = ip.src_addr().to_string();
        string dst_ip = ip.dst_addr().to_string();

        int src_port = tcp.sport();
        int dst_port = tcp.dport();

        ss << "{" <<
            "\"type\":\"get\"," <<
            parse_radiotap(packet, rt) <<
            "\"src\":{" <<
                "\"ip\":\"" << src_ip << "\"," <<
                "\"port\":" << src_port <<
                "}," <<
            "\"dst\":{" <<
                "\"ip\":\"" << dst_ip << "\"," <<
                "\"port\":" << dst_port <<
                "}," <<
            "\"host\":\"" << host << "\"," <<
            "\"get\":\"" << get << "\"" <<
            // ",\"raw\":\"" << data_printable.str() << "\"" <<
            "}";
    } catch (...) {
    }
    return ss.str();
}

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

int main(int argc, char* argv[]) {

    string interface = "en0";
    if(argc > 1){
        interface = argv[1];
    }

    SnifferConfiguration config;
    config.set_promisc_mode(true);
    config.set_rfmon(true);
    config.set_filter("type mgt or type data");

    Sniffer sniffer(interface, config);

    while(true) {
        try {
            Packet packet = sniffer.next_packet();
            if(packet) {
                // println(parse_types(packet));
                // println(parse_beacon(packet));
                // println(parse_probe(packet));
                // println(parse_ip(packet));
                println(parse_get(packet));
            }
        } catch (...) {
        }
    }
}

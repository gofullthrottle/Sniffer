#include <iostream>
#include <set>
#include <string>
#include "tins/tins.h"
#include <signal.h>
#include <unistd.h>

using namespace Tins;
using namespace std;

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

string custom_parse(const Packet& packet) {
    try {
        stringstream ss;
        const RadioTap &rt =  packet.pdu()->rfind_pdu<RadioTap>();
        ss << (unsigned int) packet.timestamp().seconds() << " ";
        ss << rt.channel_freq() << " ";
        const PDU *cur = packet.pdu();
        while(cur) {
            ss << Utils::to_string(cur->pdu_type()) << " ";
            cur = cur->inner_pdu();
        }
        return ss.str();
    } catch (...) {
        return "";
    }
}

string partial_parse_radiotap(const Packet& packet) {
    stringstream ss;

    try {
        const RadioTap &rt =  packet.pdu()->rfind_pdu<RadioTap>();

        try {
            unsigned int timestamp = packet.timestamp().seconds();
            ss << "\"timestamp\":" << timestamp;
        } catch (...) {
        }

        try {
            int dbm_signal = rt.dbm_signal();
            ss << ",\"dbm_signal\":" << dbm_signal;
        } catch (...) {
        }

        try {
            int rate = rt.rate();
            ss << ",\"rate\":" << rate;
        } catch (...) {
        }

        try {
            int channel_freq = rt.channel_freq();
            ss << ",\"channel_freq\":" << channel_freq;
        } catch (...) {
        }

    } catch (...) {
    }

    return ss.str();
}

string partial_parse_types(const Packet& packet) {
    stringstream ss;
    try {
        const PDU *cur = packet.pdu();
        ss << "\"types\":\"";
        while(cur) {
            ss << Utils::to_string(cur->pdu_type()) << " ";
            cur = cur->inner_pdu();
        }
        ss.seekp(-1, ios_base::end);
        ss << "\"";
        // ss << ",\"size\":" << packet.pdu()->size();
    } catch (...) {
    }
    return ss.str();
}

string parse_types(const Packet& packet) {
    stringstream ss;
    try {
        ss << "{" <<
            partial_parse_radiotap(packet) << "," <<
            partial_parse_types(packet) << 
            "}";
    } catch (...) {
    }
    return ss.str();
}

string parse_beacon(const Packet& packet) {
    stringstream ss;
    try {
        const Dot11Beacon &data = packet.pdu()->rfind_pdu<Dot11Beacon>();
        string ssid = data.ssid();
        HWAddress<6> src_addr = get_src_addr(data);
        bool privacy = data.capabilities().privacy();
        ss << "{" <<
            "\"type\":\"beacon\"," <<
            partial_parse_radiotap(packet) << "," <<
            "\"src_addr\":\"" << src_addr << "\"," <<
            "\"ssid\":\"" << ssid << "\"," <<
            "\"privacy\":\"" << (privacy ? "true" : "false") << "\"" <<
            "}";
    } catch (...) {
    }
    return ss.str();
}

string parse_probe(const Packet& packet) {
    stringstream ss;
    try {
        // first check that it's a probe request frame
        packet.pdu()->rfind_pdu<Tins::Dot11ProbeRequest>();
        const Dot11ManagementFrame &data = packet.pdu()->rfind_pdu<Dot11ManagementFrame>();
        string ssid = data.ssid();
        HWAddress<6> src_addr = get_src_addr(data);
        ss << "{" <<
            "\"type\":\"probe\"," <<
            partial_parse_radiotap(packet) << "," <<
            "\"src_addr\":\"" << src_addr << "\"," <<
            "\"ssid\":\"" << ssid << "\"" <<
            "}";
    } catch (...) {
    }
    return ss.str();
}

string parse_ip(const Packet& packet) {
    stringstream ss;
    try {
        // first check that we have everything we need
        const IP &ip = packet.pdu()->rfind_pdu<IP>();

        string src_ip = ip.src_addr().to_string();
        string dst_ip = ip.dst_addr().to_string();

        ss << "{" <<
            "\"type\":\"ip\"," <<
            partial_parse_types(packet) << "," <<
            partial_parse_radiotap(packet) << "," <<
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

string parse_get(const Packet& packet) {
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
        
        int end = data_str.find(" ", get_index+4);
        string get = data_str.substr(get_index+4,end-(get_index+4));
        end = data_str.find("\r", host_index+6);
        string host = data_str.substr(host_index+6,end-(host_index+6));

        const RadioTap &rt =  packet.pdu()->rfind_pdu<RadioTap>();

        ss << rt.channel_freq() << "|http://" << host << get;
        return ss.str();
        
        const IP &ip = packet.pdu()->rfind_pdu<IP>();
        const TCP &tcp = packet.pdu()->rfind_pdu<TCP>();

        string src_ip = ip.src_addr().to_string();
        string dst_ip = ip.dst_addr().to_string();

        int src_port = tcp.sport();
        int dst_port = tcp.dport();

        ss << "{" <<
            "\"type\":\"get\"," <<
            partial_parse_radiotap(packet) << "," <<
            partial_parse_types(packet) << "," <<
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
    // config.set_filter("type data or type mgt subtype probe-req or type mgt subtype beacon");
    config.set_filter("type data");

    Sniffer sniffer(interface, config);

    while(true) {
        try {
            Packet packet = sniffer.next_packet();
            if(packet) {
                // println(custom_parse(packet));
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

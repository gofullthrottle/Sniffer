// based on https://gist.github.com/pavel-a/77eb3295fcefdf2bc992
// build with:
// $ clang -framework Foundation -framework CoreWLAN channel-hop.m -o channel-hop

// this will fail if the airport power is off (-3903)
// or if you are currently monitoring 

int hopDelay = 200; // milliseconds
int n = 37;
int channelList[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,
    36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,149,153,157,161,165};
int channelWidthList[] = {20,20,20,20,20,20,20,20,20,20,20,20,20,
    80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,40,40,20,80,80,80,80,20};

#import <Foundation/Foundation.h>
#import <CoreWLAN/CoreWLAN.h>
#include <stdio.h>
#include <unistd.h>

int getkCWChannelWidth(int bandwidth) {
    switch(bandwidth) {
        case 20: return kCWChannelWidth20MHz;
        case 40: return kCWChannelWidth40MHz;
        case 80: return kCWChannelWidth80MHz;
        case 160: return kCWChannelWidth160MHz;
        default: return -1;
    }
}

int channelHop() {
    CWWiFiClient *wfc = CWWiFiClient.sharedWiFiClient;
    if (!wfc) {
        return 1;
    }

    CWInterface *wif = wfc.interface;
    if (!wif) {
        return 1;
    }
    
    [wif disassociate];

    NSSet * channels = [wif supportedWLANChannels];
    NSError *err;
    while(true) {
        for (int i = 0; i < n; i++) {
            int channel = channelList[i];
            int channelWidth = getkCWChannelWidth(channelWidthList[i]);
            bool success = false;
            for (CWChannel * channelObj in channels) {
                if ([channelObj channelWidth] == channelWidth) {
                    if([channelObj channelNumber] == channel) {
                        printf("%d,%d\n", channel, channelWidthList[i]);
                        success = [wif setWLANChannel:channelObj error:&err];
                        if(success) {
                            break;
                        }
                    }
                }
            }
            if (!success) {
                printf("Error: %ld\n", err.code);
                return 1;
            }
            usleep(hopDelay * 1000); // 1000 microseconds per millisecond
        }
    }

    return 0;
}


int main(int argc, const char * argv[])
{
    @autoreleasepool {
        fprintf(stderr, "Wi-Fi channel-hopper (ctrl-c to exit).\n");
        channelHop();
    }
    return 0;
}
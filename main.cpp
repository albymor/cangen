#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <fcntl.h>

#include "json.hpp"
#include <fstream>

#define BASE_CYCLE_TIME 1 // 1ms
#define NSEC_PER_SEC 1000000000LL
#define CLOCKFD 3
#define FD_TO_CLOCKID(fd) ((clockid_t)((((unsigned int)~fd) << 3) | CLOCKFD))

bool use_phc = false;
std::string ifname = "vcan0";       // default interface name
std::string phc_name = "/dev/ptp1"; // default PHC name

int fd_ptp;

static int clockid = CLOCK_TAI;

using json = nlohmann::json;

uint32_t get_random(uint32_t min, uint32_t max)
{
    return min + std::rand() / ((RAND_MAX + 1u) / max); // Note: 1 + rand() % 6 is biased
}

int send_thread(uint32_t rate, uint32_t can_id, uint32_t startup_delay, std::string ifname = "vcan0")
{
    int s;
    int nbytes;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;

    static struct timespec ts_send;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1)
    {
        perror("Error while opening socket");
        return -1;
    }

    strcpy(ifr.ifr_name, ifname.c_str());
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Error in socket bind");
        return -2;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(startup_delay));

    std::cout << "Thread for ID " << can_id << " with rate " << rate << "ms has started on " << ifname << std::endl;

    while (1)
    {
        frame.can_id = can_id;
        frame.can_dlc = 8;

        int ret;

        ret = clock_gettime(clockid, &ts_send);

        if (ret)
        {
            perror("clock_gettime");
            return ret;
        }
        uint64_t t_send;
        t_send = ts_send.tv_sec * NSEC_PER_SEC + ts_send.tv_nsec;
        memcpy(frame.data, &t_send, 8);

        nbytes = write(s, &frame, sizeof(struct can_frame));
        std::this_thread::sleep_for(std::chrono::milliseconds(rate));
    }

    return 0;
}

void set_timers()
{
    if (use_phc)
    {
        std::cout << "Using PHC\n";
        fd_ptp = open(phc_name.c_str(), O_RDONLY);
        if (fd_ptp < 0)
        {
            perror(phc_name.c_str());
            exit(1);
        }
        clockid = FD_TO_CLOCKID(fd_ptp);
    }
    else
    {
        std::cout << "Using TAI\n";
    }
}

void printUsage() {
  std::cout << "Usage: cangen OPTIONS" << std::endl;
  std::cout << "Available options:" << std::endl;
  std::cout << "\t -i INTERFACE \t\t can interface, default: vcan0" << std::endl;
  std::cout << "\t -f filename \t\t path to json with id and timing" << std::endl;
  std::cout << "\t -P phc_clock\t\t path to phc clock. If not provided clock TAI is used" << std::endl;
  std::cout << "\t -h \t\t\t display this help text" << std::endl;
}

int main(int argc, char *argv[])
{

    std::string filename;

    int option;

    while ((option = getopt(argc, argv, "P:i:f:h")) != -1)
    {
        switch (option)
        {
        case 'P':
            use_phc = true;
            phc_name = optarg;
            break;
        case 'i':
            ifname = optarg;
            break;
        case 'f':
            filename = optarg;
            std::cout << "Filename: " << optarg << std::endl;
            break;
        case 'h':
        default:
            printUsage();
            return 1;
        }
    }

    if (filename.empty())
    {
        std::cerr << "Filename not provided" << std::endl;
        printUsage();
        return 1;
    }

    std::ifstream f(filename); // Use the filename from the command line
    if (!f.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    json data = json::parse(f);

    set_timers();

    std::srand(std::time(nullptr)); // use current time as seed for random generator

    std::vector<std::thread> threads;

    for (json::iterator it = data.begin(); it != data.end(); ++it)
    {
        uint32_t id = std::stoi(it.key());
        uint32_t rate = it.value();
        threads.emplace_back(send_thread, rate, id, get_random(0, 100), ifname);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    return 0;
}

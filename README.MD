# Cangen - CAN Message Generator

Cangen is a flexible CAN message generator that allows users to simulate CAN bus traffic with specified message IDs and timings. It supports both the Transmission Time (TAI) and the Precision Time Protocol (PHC) clocks for precise timing requirements. Notably, cangen embeds the creation time of each CAN frame directly into the frame data. The program is specifically crafted for measuring the latency of CAN frames transmitted through CAN-Ethernet gateways, offering a valuable tool for assessing communication performance in such scenarios.

## Prerequisites

- Linux environment
- SocketCAN kernel module
- C++ compiler with C++11 support

## Dependencies

Cangen only depends from https://github.com/nlohmann/json. All the files necessary to compile cangen are already present in this repository.

## Installation

No special installation is required. Only CMake is required.

```bash
mkdir build
cd build 
cmake ..
make
```

## Usage

```bash
./cangen -i INTERFACE -f filename [-P phc_clock] [-h]
```

**Options:**
- `-i INTERFACE`: Specifies the CAN interface to use (default: vcan0).
- `-f filename`: Path to a JSON file containing CAN message IDs and corresponding timings.
- `-P phc_clock`: Path to the PHC clock. If not provided, TAI clock is used.
- `-h`: Display help text.

## Example

```bash
./cangen -i can0 -f messages.json -P /dev/ptp1
```

This command will start generating CAN messages on the `can0` interface based on the configuration provided in the `messages.json` file. The Precision Time Protocol (PHC) clock located at `/dev/ptp1` will be used for precise timing.

## JSON Configuration

The JSON file should have the following format:

```json
{
  "CAN_ID_1": "TIMING_1",
  "CAN_ID_2": "TIMING_2",
  ...
}
```

- `CAN_ID_x`: CAN message ID in decimal format.
- `TIMING_x`: Timing in milliseconds for the corresponding CAN message ID.

## Features

- Supports both TAI and PHC clocks.
- Configurable through a JSON file.
- Simulates CAN traffic with specified message IDs and timings.

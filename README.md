# XP12_A330_Skalarki
Connect Skalarki desktop line FCU to default A330 in XP12:
https://www.skalarki-electronics.com/en/onlineshop/product/desktopline-fcu.html

## How to use it

- Copy plugin into "X-Plane 12/Aircraft/Laminar Research/Airbus A330-300/plugins" folder
- Configure Skalarki profiler for use with X-Plane (Toliss aircraft).
    - IP address must be 127.0.0.1
    - Port must be 53000.
- Start SDK mode.
- Set FCU switches into default positions:
    - CPT Baro = in Hg
    - CPT ND = LS
    - CPT ND Range = 40 nm
    - ALt mode = 100
- Start X-Plane and load aircraft

## Test

- Only tested with my own hardware right now. There might be different versions (different firmware version?) which might not work with this plugin.
- Tested with Skalarki IO Profiler 5.1.110.1000.

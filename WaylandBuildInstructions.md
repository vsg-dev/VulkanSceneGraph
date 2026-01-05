**Prequesites:**
* xkbcommon and wayland development libraries must be installed
 * sudo apt install libxkbcommon-dev libwayland-dev wayland-protocols

**Build steps**
* Checkout wayland-testing branch
 * git checkout wayland-testing
* Run following commands to build
 * mkdir build
 * cd build
 * cmake .. -DBUILD_WAYLAND=1
 * make -jx (where x is number of cores)
* It is based on version 1.1.0 for now..

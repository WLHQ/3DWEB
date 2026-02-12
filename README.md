# 3DWEB - High-Performance HTTP Server for Nintendo 3DS

3DWEB is a robust, multi-threaded HTTP server written in C, specifically optimized for the Nintendo 3DS hardware. It transforms your handheld console into a capable web server, supporting dynamic content, file serving, and system interactions.

This project is a heavily modified and modernized continuation of `3ds-httpd` by dimaguy.


## Features

*   **Zero-Config Setup:** Automatically creates a `/Websites/` directory on the SD card root with a default `index.html` on first launch.
*   **SD Card Serving:** Serves static files directly from the SD card.
*   **System Tools:** Remote reboot and exit functionality.
*   **Native Encryption:** Exposes the 3DS's hardware AES engine for experimental crypto operations (CBC/CTR/CTM).
*   **Memory Operations:** (Experimental) Read/Write access to system memory via HTTP endpoints.

## Installation & Usage

1.  **Download:** Get the latest `3DWEB.3dsx` from the releases page (or build it yourself).
2.  **Install:** Copy `3DWEB.3dsx` to the `/3ds/` folder on your SD card.
3.  **Setup Website:**
    *   Create a folder named `Websites` on the root of your SD card (`sdmc:/Websites/`).
    *   Place your `index.html` and other assets (css, js, images) inside this folder.
    *   *Note:* If the folder doesn't exist, 3DWEB will create it with a demo page on first start.
4.  **Run:** Launch **3DWEB** via the Homebrew Launcher.
5.  **Connect:** Open a web browser on a PC or phone connected to the same Wi-Fi and navigate to the IP address displayed on the 3DS screen (e.g., `http://192.168.x.x`).

## API Endpoints & Handlers

The server exposes several built-in endpoints:

### 📂 File Server
*   **`/`**: Serves `index.html` from `/Websites/`.
*   **`/sdcard/*`**: Browses and serves files relative to the SD card root.
    *   Example: `http://3ds-ip/sdcard/luma/config.ini`

### ⚙️ System Control
*   **`/system/exit`**: Closes the 3DWEB application and returns to Homebrew Launcher.
*   **`/system/reboot`**: Performs a full system reboot.

### 🛠️ Advanced Tools
*   **`/crypt/`**: Interfaces with the 3DS hardware AES engine.
*   **`/readmem/` & `/writemem/`**: Direct memory access (Use with extreme caution! Can crash the system).

## Building

To build from source, you need the **devkitPro** environment with `devkitARM` and `libctru` installed.

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/3DS.cmake ..
make
```

## Credits

*   **Original Author:** [dimaguy](https://github.com/dimaguy/3ds-httpd)
*   **ME:** [Rawrvlxx](https://github.com/settledinmisery-spec)

Licensed under the MIT License.

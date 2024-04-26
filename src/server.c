#include "httpserver.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h> // Include for chdir function

static u32 *socket_buffer = NULL;
static http_server data;
http_server *app_data = &data;
static int ret;
static char payload[4098];
PrintConsole topScreen, bottomScreen;

void socShutdown()
{
    printTop("waiting for socExit...\n");
    socExit();
}

void init(int port)
{
    hidInit(); // input
    psInit(); // ps, for AES
    gfxInitDefault(); // graphics
    consoleInit(GFX_TOP, &topScreen);
    consoleInit(GFX_BOTTOM, &bottomScreen);
    fsInit();
    consoleDebugInit(debugDevice_CONSOLE);
    init_handlers();
    socket_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    ndmuInit();
    aptSetSleepAllowed(false);
    Result res;
    if(R_FAILED(res = NDMU_EnterExclusiveState(NDM_EXCLUSIVE_STATE_INFRASTRUCTURE)))
        failExit("Failed to enter NDMU Exclusive State");
    NDMU_LockState();
    if (socket_buffer == NULL)
        failExit("Socket buffer allocation failed!\n");

    // Init soc:u service
    if ((ret = socInit(socket_buffer, SOC_BUFFERSIZE)) != 0)
        failExit("Service initialization failed! (code: 0x%08X)\n", (unsigned int)ret);

    // Make sure the struct is clear
    memset(&data, 0, sizeof(data));
    data.client_id = -1;
    data.server_id = -1;
    data.server_id = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    // Is socket accessible?
    if (data.server_id < 0)
        failExit("socket: %s (code: %d)\n", strerror(errno), errno);

    // Init server_addr on default address and port 8081
    data.server_addr.sin_family = AF_INET;
    data.server_addr.sin_port = htons(port);
    data.server_addr.sin_addr.s_addr = gethostid();
    data.client_length = sizeof(data.client_addr);

    // Create directory and index.html file on SD card
    const char *directory = "Websites"; // Change this to your desired directory
    const char *index_html = "index.html"; // Change this to your desired index.html path

    // Check if the directory already exists
    struct stat dir_stat;
    if (stat(directory, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode)) {
        printTop("Directory already exists: %s\n", directory);
    } else {
        // Directory does not exist, so create it
        printTop("Creating directory: %s\n", directory);
        if (mkdir(directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            if (errno != EEXIST) {
                failExit("Failed to create directory: %s (code: %d)\n", strerror(errno), errno);
            }
        } else {
            printTop("Directory created: %s\n", directory);
        }
    }

    // Change the current working directory to the Websites folder
    if (chdir(directory) != 0) {
        failExit("Failed to change directory to %s: %s (code: %d)\n", directory, strerror(errno), errno);
    }

    // Check if the index.html file already exists
    if (stat(index_html, &dir_stat) == 0 && S_ISREG(dir_stat.st_mode)) {
        printTop("index.html file already exists.\n");
    } else {
        // index.html file does not exist, so create it
        printTop("Creating index.html file: %s\n", index_html);
        FILE *index_file = fopen(index_html, "w");
        if (index_file == NULL) {
            failExit("Failed to create index.html file: %s (code: %d)\n", strerror(errno), errno);
        } else {
            const char *html_content = "<html><head><title>Index</title></head><body><h1>Welcome to my website! Place your own index.html file in the Websites directory located on the root of your 3DS SD Card.</h1></body></html>";
            fputs(html_content, index_file);
            fclose(index_file);
            printTop("index.html file created: %s\n", index_html);
        }
    }

    // Print network info
    printTop("Server is starting - http://%s:%i/\n", inet_ntoa(data.server_addr.sin_addr),port);

    if ((ret = bind(data.server_id, (struct sockaddr *) &data.server_addr, sizeof(data.server_addr))))
    {
        close(data.server_id);
        failExit("bind: %s (code: %d)\n", strerror(errno), errno);
    }

    // Set socket non blocking so we can still read input to exit
    fcntl(data.server_id, F_SETFL, fcntl(data.server_id, F_GETFL, 0) | O_NONBLOCK);

    if ((ret = listen(data.server_id, 5)))
        failExit("listen: %s (code: %d)\n", strerror(errno), errno);
    data.running = 1;
    printTop("Ready...\n");
}

int loop()
{
    data.client_id = accept(data.server_id, (struct sockaddr *) &data.client_addr, &data.client_length);
    if (data.client_id < 0 && errno != EAGAIN)
        failExit("accept: %d %s\n", errno, strerror(errno));
    else
    {
        // set client socket to blocking to simplify sending data back
        fcntl(data.client_id, F_SETFL, fcntl(data.client_id, F_GETFL, 0) & ~O_NONBLOCK);
        // reset old payload
        memset(payload, 0, 4098);

        // Read 1024 bytes (FIXME: dynamic size)
        ret    = recv(data.client_id, payload, 4096, 0);

        // HTTP 1.1?
        if (strstr(payload, "HTTP/1.1"))
            manage_connection(&data, payload);

        // End connection
        close(data.client_id);
        data.client_id = -1;
    }
    return data.running;
}

void destroy()
{
    NDMU_UnlockState();
    NDMU_LeaveExclusiveState();
    aptSetSleepAllowed(true);
    ndmuExit();
    close(data.server_id);
    socShutdown();
    gfxExit();
    hidExit();
    psExit();
}

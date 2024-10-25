#define ANALOGSDK_EXPORTS

#include "plugin.h"
#include "string.h"
#include "hidapi.h"

//These are required for linking to wooting_analog_plugin_dev on Windows
#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "ntdll.lib")

#define ANALOG_BUFFER_SIZE 48
#define WOOTING_VID 0x31E3
#define WOOTING_ANALOG_USAGE_PAGE 0xFF54

static hid_device* keyboard_handle = NULL;
static void const* callback_data = NULL;
static device_event callback = NULL;
static unsigned char hid_read_buffer[ANALOG_BUFFER_SIZE];

static char device_name[64];
static char manufacturer_name[64];

static WootingAnalog_DeviceInfo_FFI dev_info;
static bool initialised = false;

const char* name(){
    return "C Test Plugin";
}

bool is_initialised() {
    return initialised;
}

static void wooting_keyboard_disconnected() {
    hid_close(keyboard_handle);
    keyboard_handle = NULL;

    if (callback) {
        callback(callback_data, WootingAnalog_DeviceEventType_Disconnected, &dev_info);
    }
    initialised = false;
}

static bool wooting_find_keyboard() {
    struct hid_device_info* hid_info = hid_enumerate(WOOTING_VID, 0);

    if (hid_info == NULL) {
        return false;
    }

    // The amount of interfaces is variable, so we need to look for the analog interface
    // In the Wooting one keyboard the analog interface is always the highest number
    struct hid_device_info* hid_info_walker = hid_info;
    uint8_t interfaceNr = 0;
    while (hid_info_walker) {
        //printf("%d\n", hid_info_walker->interface_number);
        if (hid_info_walker->interface_number > interfaceNr) {
            interfaceNr = hid_info_walker->interface_number;
        }
        hid_info_walker = hid_info_walker->next;
    }

    bool keyboard_found = false;
    // Reset walker to top and search for the interface number
    hid_info_walker = hid_info;
    while (hid_info_walker) {
        if (hid_info_walker->interface_number == interfaceNr) {
            keyboard_handle = hid_open_path(hid_info_walker->path);
            if (keyboard_handle) {
                keyboard_found = true;
            }

            break;
        }

        hid_info_walker = hid_info_walker->next;
    }

    if (keyboard_found) {
        dev_info.vendor_id = hid_info->vendor_id;
        dev_info.product_id = hid_info->product_id;
        sprintf(device_name, "%ls", hid_info->product_string );
        dev_info.device_name = device_name;
        sprintf(manufacturer_name, "%ls", hid_info->manufacturer_string );
        dev_info.manufacturer_name = manufacturer_name;
        char serial[40];
        sprintf(serial, "%ls", hid_info->serial_number);

        dev_info.device_id = generate_device_id(serial, hid_info->vendor_id, hid_info->product_id);
    }

    hid_free_enumeration(hid_info);
    return keyboard_found;
}

WootingAnalogResult initialise(void const* cb_data, device_event cb) {
    if (initialised)
        return WootingAnalogResult_Ok;

    callback_data = cb_data;
    callback = cb;

    return initialised = wooting_find_keyboard();
}

void unload() {

}

static bool wooting_refresh_buffer() {
    if (!keyboard_handle) {
        if (!wooting_find_keyboard()) {
            return false;
        }
    }

    int hid_res = hid_read_timeout(keyboard_handle, hid_read_buffer, ANALOG_BUFFER_SIZE, 0);

    // If the read response is -1 the keyboard is disconnected
    if (hid_res == -1) {
        wooting_keyboard_disconnected();
        return false;
    }
    else {
        return true;
    }
}

int read_full_buffer(uint16_t code_buffer[], float analog_buffer[], int len, WootingAnalog_DeviceID device) {
    if (!initialised)
        return (float)WootingAnalogResult_UnInitialized;

    if (device != 0 && dev_info.device_id != device)
        return (float)WootingAnalogResult_NoDevices;

    if (!wooting_refresh_buffer())
        return (float)WootingAnalogResult_DeviceDisconnected;


    int items_written = 0;
    int read_length = len;

    // Cap elements to read
    if (len > ANALOG_BUFFER_SIZE) {
        read_length = ANALOG_BUFFER_SIZE/3;
    }

    for (int i = 0; i < read_length*3; i += 3) {
        uint16_t code = (hid_read_buffer[i] << 8) | hid_read_buffer[i+1];
        uint8_t analog_value = hid_read_buffer[i + 2];

        if (analog_value > 0) {
            code_buffer[items_written] = code;

            analog_buffer[items_written] = (float)analog_value / 255.0f;

            items_written++;
        }
        else {
            // There will be no other keys once an analog value is 0
            return items_written;
        }
    }

    return items_written;
}

float read_analog(uint16_t code, WootingAnalog_DeviceID device) {
    if (!initialised)
        return (float)WootingAnalogResult_UnInitialized;

    if (device != 0 && dev_info.device_id != device)
        return (float)WootingAnalogResult_NoDevices;

    if (!wooting_refresh_buffer())
        return (float)WootingAnalogResult_DeviceDisconnected;



    for (int i = 0; i < ANALOG_BUFFER_SIZE && hid_read_buffer[i+2] > 0; i += 3) {
        uint16_t read_code = (hid_read_buffer[i] << 8) | hid_read_buffer[i+1];
        if (read_code == code) {
            return (float)hid_read_buffer[i + 2] / 255.0f;
        }
    }

    return 0.0f;
}

int device_info(WootingAnalog_DeviceInfo_FFI* buffer[], int len) {
    if (!initialised)
        return WootingAnalogResult_UnInitialized;

    buffer[0] = &dev_info;
    return 1;
}
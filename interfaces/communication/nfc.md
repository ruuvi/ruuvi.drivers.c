# NFC Driver
Ruuvi NFC driver must implement the flow described below. 

## Usage

### Configure
Configuration file `application_config.h` must specify these fields:
```
#define APPLICATION_NFC
#define NFC_TEXT_BUF_SIZE 128
#define NFC_URI_BUF_SIZE 64
#define NFC_APP_BUF_SIZE 64
#define NFC_DATA_BUF_SIZE 240
#define NDEF_FILE_SIZE 512
#define NFC_MAX_NUMBER_OF_RECORDS 4
```

`NDEF_FILE_SIZE` Should be a bit larger than the rest of fields combined. 
Data buffer will be initialized twice, once for tx data and once for rx data
Above example would therefore use `128 + 64 + 64 + 2*240 + 512 = 1248` bytes of memory.
Additionally some memory is required by static state variables. 

### Initialize
Setup function pointers, i.e.
```
  ruuvi_communication_channel_t nfc;
  nfc.init = nfc_init;
  nfc.uninit = nfc_uninit;
  nfc.message_get = nfc_message_get;
  nfc.process_asynchronous = nfc_process_asynchronous;
```
Call `nfc.init()`

### Setup transmitted data
NFC splits data into records. It's possible to set up static application data, such as self-test status and tag ID
as well as URL for more information. Call
```
nfc_text_record_set(const uint8_t* text, size_t length);
nfc_uri_record_set(const uint8_t* uri, size_t length);
nfc_app_record_set(const uint8_t* app, size_t length);
```
as appropriate. URI Starts always with `https://`, write `ruuvi.com` with length `9` to setup `https://ruuvi.com`
Application record is for Android only at this time. 

TODO: Call `nfc_message_put(ruuvi_communication_message_t* msg);` to setup application data payload.
If repeat flag of message is set, the data is automatically restored after incoming data write, i.e. 
tag becomes unwriteable and write operations will only transmit data from user to application. 

Currently putting custom binary messages is not yet supported.

If repeat flag is not set, data written by user will overwrite the data on tag, i.e. tag becomes user writeable. 
#### Load data from flash - on roadmap

### Process data
Call `nfc.process_asynchronous();`
This saves received data into SW buffer and moves records to be transmitted from SW buffers into NFC buffer.

### Parse received data
Data is received as NDEF records. Payload of each record will be parsed into a separate message. 
Note: Some records, such as text record, will have type data in payload. For example tex record can have a 3-byte header.
```
    ruuvi_communication_message_t rx;
    uint8_t payload[20];
    rx.payload = payload;
    rx.payload_length = sizeof(payload);
    while(RUUVI_SUCCESS == nfc.message_get(&rx))
    {
      PLATFORM_LOG_INFO("Got message");
      PLATFORM_LOG_HEXDUMP_INFO(rx.payload, rx.payload_length);
      // Payload length gets overwritten, let the driver know full length of buffer
      rx.payload_length = sizeof(payload);
    }
```

### Update transmitted data if necessary - on roadmap

### Store data to flash - on roadmap

### Uninitialize
Call `nfc.uninit();`
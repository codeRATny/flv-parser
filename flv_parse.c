/*
===================================INFO===================================
    Packet_Type:
        8 - audio
        9 - video
        18 - script data
===================================INFO===================================
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <alloca.h>

#define SIZEOF_SIGNATURE 3
#define SIZEOF_PAYLOADSIZE 3
#define SIZEOF_TIMESTAMPLOWER 3
#define SIZEOF_STREAMID 3
#define SIZEOF_FLV_HEADER SIZEOF_SIGNATURE + 1 + 1 + __SIZEOF_INT__
#define SIZEOF_FLV_PACK_HEADER __SIZEOF_INT__ + 1 + SIZEOF_PAYLOADSIZE + SIZEOF_TIMESTAMPLOWER + 1 + SIZEOF_STREAMID

#define OFFSET_HEADER_SIZE_BE SIZEOF_SIGNATURE + 1 + 1
#define OFFSET_HEADER_VERSION SIZEOF_SIGNATURE
#define OFFSET_HEADER_FLAGS SIZEOF_SIGNATURE + 1
#define OFFSET_PACK_PAYLOADSIZE_BE __SIZEOF_INT__ + 1, SIZEOF_PAYLOADSIZE
#define OFFSET_PACK_STREAMIDBE __SIZEOF_INT__ + 1 + SIZEOF_PAYLOADSIZE + SIZEOF_TIMESTAMPLOWER + 1
#define OFFSET_PACK_TIMESTAMPLOWER_BE __SIZEOF_INT__ + 1 + SIZEOF_PAYLOADSIZE
#define OFFSET_PACK_TIMESTAMPUPPER __SIZEOF_INT__ + 1 + SIZEOF_PAYLOADSIZE + SIZEOF_TIMESTAMPLOWER

// 0 - Big Endian
// 1 - Little Endian
uint8_t get_byte_order()
{
    uint16_t check = 1;
    return *((char *)(&check));
}

void swap_byte_order(void *dest, void *src, size_t size)
{
    for (size_t i = size - 1; i + 1 > 0; i--)
    {
        *(((uint8_t *)dest) + i) = *(((uint8_t *)src) + size - 1 - i);
    }
}

void parse_flv_header(int fd)
{
    uint32_t header_size_le;
    uint8_t *data = alloca(SIZEOF_FLV_HEADER);

    read(fd, data, SIZEOF_FLV_HEADER);

    swap_byte_order(&header_size_le, (data + OFFSET_HEADER_SIZE_BE), __SIZEOF_INT__);

    printf("Signature = %c%c%c\n", *(data), *(data + 1), *(data + 2));
    printf("Version = %d\n", *(data + OFFSET_HEADER_VERSION));
    printf("Flags = %d\n", *(data + OFFSET_HEADER_FLAGS));
    printf("Header size = %d\n\n", header_size_le);
}

ssize_t parse_flv_pack(int fd)
{
    uint32_t payload_size_le = 0;
    uint32_t size_prev_le = 0;
    uint32_t streamid_le = 0;
    uint32_t timestamplower_le = 0;

    uint8_t *data = alloca(SIZEOF_FLV_PACK_HEADER);

    ssize_t read_result = read(fd, data, SIZEOF_FLV_PACK_HEADER);

    if ((read_result == 0) || (read_result == -1))
    {
        return read_result;
    }

    swap_byte_order(&payload_size_le, data + OFFSET_PACK_PAYLOADSIZE_BE);

    uint8_t *Payload_Data = alloca(payload_size_le);
    read(fd, Payload_Data, payload_size_le);

    swap_byte_order(&size_prev_le, data, __SIZEOF_INT__);
    swap_byte_order(&timestamplower_le, data + OFFSET_PACK_TIMESTAMPLOWER_BE, SIZEOF_TIMESTAMPLOWER);
    swap_byte_order(&streamid_le, data + OFFSET_PACK_STREAMIDBE, SIZEOF_STREAMID);

    printf("Size of previous packet = %d\n", size_prev_le);
    printf("Packet type = %d\n", *(data + __SIZEOF_INT__));
    printf("Payload size = %d\n", payload_size_le);
    printf("Timestamp lower = %d\n", timestamplower_le);
    printf("Timestamp upper = %d\n", *(data + OFFSET_PACK_TIMESTAMPUPPER));
    printf("Stream ID = %d\n\n", streamid_le);

    return read_result;
}

int main()
{
    int fd = open("sample.flv", O_RDONLY);

    parse_flv_header(fd);
    while (parse_flv_pack(fd))
        ;

    return 0;
}
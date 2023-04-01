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

// read(fd, header.Signature, SIZEOF_SIGNATURE);
// read(fd, &header.Version, 1);
// read(fd, &header.Flags, 1);
// read(fd, &header.Header_Size_be, __SIZEOF_INT__);
// swap_byte_order(&le, &header.Header_Size_be, sizeof(le));
// printf("Signature = %c%c%c\n", header.Signature[0], header.Signature[1], header.Signature[2]);
// printf("Version = %d\n", header.Version);
// printf("Flags = %d\n", header.Flags);
// printf("Header size = %d\n\n", le);

// read(fd, &pack.Size_Of_Previous_Packet_be, 4);
// read(fd, &pack.Packet_Type, 1);
// read(fd, &pack.Payload_Size_be, 3);
// read(fd, &pack.Timestamp_Lower_be, 3);
// read(fd, &pack.Timestamp_Upper, 1);
// read(fd, &pack.Stream_ID_be, 3);

// swap_byte_order(&size_le, &pack.Payload_Size_be, 3);
// swap_byte_order(&size_prev_le, &pack.Size_Of_Previous_Packet_be, sizeof(size_prev_le));
// swap_byte_order(&streamid_le, &pack.Stream_ID_be, 3);
// printf("Size_Of_Previous_Packet = %d\n", size_prev_le);
// printf("Packet_Type = %d\n", pack.Packet_Type);
// printf("Payload_Size = %d\n", size_le);
// printf("Timestamp_Lower = %d\n", *(uint32_t *)pack.Timestamp_Lower_be);
// printf("Timestamp_Upper = %d\n", pack.Timestamp_Upper);
// printf("Stream_ID = %d\n\n", streamid_le);

// typedef struct flv_header
// {
//     int8_t Signature[SIZEOF_SIGNATURE]; // Little Endian
//     uint8_t Version;                    // Little Endian
//     uint8_t Flags;                      // Little Endian
//     uint32_t Header_Size_be;            // Big Endian
// } flv_header;

// typedef struct flv_pack
// {
//     uint32_t Size_Of_Previous_Packet_be;               // Big Endian
//     uint8_t Packet_Type;                               // Little Endian
//     uint8_t Payload_Size_be[SIZEOF_PAYLOADSIZE];       // Big Endian
//     uint8_t Timestamp_Lower_be[SIZEOF_TIMESTAMPLOWER]; // Big Endian
//     uint8_t Timestamp_Upper;                           // Little Endian
//     uint8_t Stream_ID_be[SIZEOF_STREAMID];             // Big Endian
//     uint8_t *Payload_Data;                             // data
// } flv_pack;

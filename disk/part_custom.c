#include <common.h>
#include <command.h>
#include <malloc.h>
#include <blk.h>
#include <string.h>
#include <stdio.h>
#include <part.h>


#define PARTITION_COUNT 6


struct partition_entry {
    char name[PART_NAME_LEN];
    unsigned long offset;
    unsigned long size;
};

static const struct partition_entry partitions[] = {
    {"os-reserved",     4 * 0x100000, 124 * 0x100000},  // 4MB - 124MB
    {"os-bootstate",    128 * 0x100000, 2 * 0x100000},  // 128MB - 2MB
    {"os-systemA",      130 * 0x100000, 256 * 0x100000},  // 130MB - 256MB
    {"os-systemB",      386 * 0x100000, 256 * 0x100000},  // 386MB - 256MB
    {"os-overlay",      642 * 0x100000, 64 * 0x100000},  // 642MB - 64MB
    {"os-reserved_end", 706 * 0x100000, 318 * 0x100000}   // 706MB - 318MB
};

static int part_get_info_custom(struct blk_desc *desc, int part,
    struct disk_partition *info)
{
    if (part < 1 || part > (sizeof(partitions) / sizeof(partitions[0]))) {
    return -1;
    }
    const struct partition_entry *p = &partitions[part - 1];

    memset(info, 0, sizeof(*info));

    info->start = p->offset / desc->blksz;
    info->size = p->size / desc->blksz;
    info->blksz = desc->blksz;

    strncpy((char *)info->name, p->name, PART_NAME_LEN);
    strncpy((char *)info->type, "Linux", PART_TYPE_LEN);

    printf ("part: %u\n", part);
    printf ("Partition Name: %s\n", info->name);
    printf ("Partition Type: %s\n", info->type);
    printf ("Start: %lu blocks\n", info->start);
    printf ("Size: %lu blocks\n", info->size);
    printf ("Block Size: %lu bytes\n", info->blksz);

    return 0;
}

static int part_test_custom(struct blk_desc *desc)
{

    if (!desc || desc->blksz == 0 || desc->lba == 0) {
        printf("ERROR: Invalid block device\n");
        return -1;
    }

    unsigned long min_required_size = partitions[PARTITION_COUNT - 1].offset +
                                      partitions[PARTITION_COUNT - 1].size;

    if (desc->lba * desc->blksz < min_required_size) {
        printf("ERROR: Device too small, available size: 0x%lx\n",
               desc->lba * desc->blksz);
        return -1;
    }
    return 0;
}

U_BOOT_PART_TYPE(custom_part) = {
    .name       = "Custom Partitions",
    .max_entries= PARTITION_COUNT,
    .get_info   = part_get_info_ptr(part_get_info_custom),
    .test       = part_test_custom,
};
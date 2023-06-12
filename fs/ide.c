/*
 * operations on IDE disk.
 */

#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>
#include "serv.h"

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in
// 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET',
//  'DEV_DISK_OPERATION_READ', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS',
//  'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void* dst, u_int nsecs) {
    u_int begin = secno * BY2SECT;
    u_int end = begin + nsecs * BY2SECT;

    //
    u_int cur_off = 0;
    u_int read_tag = 0;
    //
    for (u_int off = 0; begin + off < end; off += BY2SECT) {
        // uint32_t temp = diskno;
        cur_off = begin + off;
        /* Exercise 5.3: Your code here. (1/2) */
        if (syscall_write_dev((void*)&diskno, DEV_DISK_ADDRESS + DEV_DISK_ID,
                              4) < 0) {
            user_panic("diskno ide_read panic");
        }
        if (syscall_write_dev((void*)&cur_off,
                              DEV_DISK_ADDRESS + DEV_DISK_OFFSET, 4) < 0) {
            user_panic("cur_off ide_read panic");
        }
        if (syscall_write_dev((void*)&read_tag,
                              DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION,
                              4) < 0) {
            user_panic("read_tag ide_read panic");
        }
        u_int status = 0;
        if (syscall_read_dev((void*)&status, DEV_DISK_ADDRESS + DEV_DISK_STATUS,
                             4) < 0) {
            user_panic("status ide_read panic");
        }
        if (!status) {
            user_panic("status error ide_read panic");
        }
        if (syscall_read_dev((void*)(dst + off),
                             DEV_DISK_ADDRESS + DEV_DISK_BUFFER,
                             DEV_DISK_BUFFER_LEN) < 0) {
            user_panic("dst ide_read panic");
        }
        //
    }
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in
// 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void* src, u_int nsecs) {
    u_int begin = secno * BY2SECT;
    u_int end = begin + nsecs * BY2SECT;
    //
    u_int cur_off = 0;
    u_int write_tag = 1;
    //
    for (u_int off = 0; begin + off < end; off += BY2SECT) {
        // uint32_t temp = diskno;
        /* Exercise 5.3: Your code here. (2/2) */
        cur_off = begin + off;
        if (syscall_write_dev((void*)&diskno, DEV_DISK_ADDRESS + DEV_DISK_ID,
                              4) < 0) {
            user_panic("diskno ide_write panic");
        }
        if (syscall_write_dev((void*)&cur_off,
                              DEV_DISK_ADDRESS + DEV_DISK_OFFSET, 4) < 0) {
            user_panic("cur_off ide_write panic");
        }
        if (syscall_write_dev((void*)(src + off),
                              DEV_DISK_ADDRESS + DEV_DISK_BUFFER,
                              DEV_DISK_BUFFER_LEN) < 0) {
            user_panic("src ide_write panic");
        }
        if (syscall_write_dev((void*)&write_tag,
                              DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION,
                              4) < 0) {
            user_panic("read_tag ide_write panic");
        }
        u_int status = 0;
        if (syscall_read_dev((void*)&status, DEV_DISK_ADDRESS + DEV_DISK_STATUS,
                             4) < 0) {
            user_panic("status ide_write panic");
        }
        if (status == 0) {
            user_panic("status error ide_write panic");
        }
        //
    }
}

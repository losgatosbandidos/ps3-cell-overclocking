// This is a modification of @aomsin2526's work - https://github.com/aomsin2526
// Many thanks to him and https://github.com/sagemono for all their help and guidance!

#include <lv2/io.h>
#include <lv2/symbols.h>
#include <lv2/error.h>
#include <lv2/libc.h>

#include <lv1/patch.h>

#include "common.h"
#include "tbfreq.h"

uint64_t _target_tbfreq = 0;

uint64_t calc_target_tbfreq()
{
    // base = 79800000
    uint64_t targetInMhz = 0;
    
    uint64_t refDiv;
    refDiv = lv1_peekd(0x20000509890UL);
    if(refDiv == 0x4d) refDiv = 5;
    else if(refDiv == 0x27) refDiv = 6;
    else if(refDiv == 0x13) refDiv = 7;
    else if(refDiv == 0x09) refDiv = 8;
    else if(refDiv == 0x84) refDiv = 9;
    else if(refDiv == 0x42) refDiv = 10;
    else if(refDiv == 0xFF) refDiv = 1;
    else if(refDiv == 0x7F) refDiv = 2;
    else if(refDiv == 0x3F) refDiv = 3;
    else if(refDiv == 0x9F) refDiv = 4;
    else refDiv = 5; // default to this even if wrong

    uint64_t current_nclk;
    current_nclk = lv1_peekd(0x3F78UL);

    uint64_t calculated_ref_clk = (uint64_t)((double)current_nclk / 79800000 * 10000000);
    uint64_t corrected_ref_clk = calculated_ref_clk * 399/400;
    targetInMhz = corrected_ref_clk / refDiv;

//debug - write to file
/*	
    const char filename[] = "/dev_usb000/log.txt";
    char buffer[500];

    int fd, ret;
    uint64_t len, nbytes;

	   ret = cellFsOpen(filename,
                     CELL_FS_O_WRONLY | CELL_FS_O_TRUNC | CELL_FS_O_CREAT,
                     &fd, 0666, NULL, 0);
					 
    len = lv2_snprintf(buffer, sizeof(buffer), "refDiv: %lu\n", refDiv);
    ret = cellFsWrite(fd, buffer, len, &nbytes);
    if(ret == CELL_FS_SUCCEEDED && len == nbytes) {
        //we printed the refdiv
    }

    len = lv2_snprintf(buffer, sizeof(buffer), "current_nclk: %lu\n", current_nclk);
    ret = cellFsWrite(fd, buffer, len, &nbytes);
    if(ret == CELL_FS_SUCCEEDED && len == nbytes) {
        //we printed the nclk
    }
	
	   len = lv2_snprintf(buffer, sizeof(buffer), "calculated_ref_clk: %lu\n", calculated_ref_clk);
    ret = cellFsWrite(fd, buffer, len, &nbytes);
    if(ret == CELL_FS_SUCCEEDED && len == nbytes) {
        //we printed the calculated_ref_clk
    }
	
	   len = lv2_snprintf(buffer, sizeof(buffer), "corrected_ref_clk: %lu\n", corrected_ref_clk);
    ret = cellFsWrite(fd, buffer, len, &nbytes);
    if(ret == CELL_FS_SUCCEEDED && len == nbytes) {
        //we printed the corrected_ref_clk
    }
	
	   len = lv2_snprintf(buffer, sizeof(buffer), "candidateTargetInMhz: %lu\n", targetInMhz);
    ret = cellFsWrite(fd, buffer, len, &nbytes);
    if(ret == CELL_FS_SUCCEEDED && len == nbytes) {
        //we printed the candidateTargetInMhz
    }
    
	   cellFsClose(fd);
*/
    if (targetInMhz != 0)
        //return ((79800000UL * ((targetInMhz * 10000UL) / 3200UL)) / 10000UL);
        return targetInMhz;

    return 0;
}

void tbfreq_apply_to_kernel(uint64_t value)
{
    if (value == 0)
        return;

    volatile uint64_t* tbfreq = (volatile uint64_t*)MKA(tbfreq_offset);
    *tbfreq = value;
}

void tbfreq_apply_to_memory(uint64_t value, void* ptr, uint64_t size)
{
    if (value == 0)
        return;

    {
        uint32_t* p = (uint32_t*)ptr;

        for (uint64_t i = 0; i < (size / 4); ++i)
        {
            if (p[i] == 79800000)
                p[i] = value;
        }
    }

    {
        float* p = (float*)ptr;

        for (uint64_t i = 0; i < (size / 4); ++i)
        {
            if (p[i] == 79800000.f)
                p[i] = (float)value;
        }
    }

    {
        double* p = (double*)ptr;

        for (uint64_t i = 0; i < (size / 8); ++i)
        {
            if (p[i] == 79800000.)
                p[i] = (double)value;
        }
    }
}

void tbfreq_apply_to_lv1(uint64_t value)
{
    if (value == 0)
        return;

    uint64_t b = ((value * 10000UL) / 79800000UL);

    uint64_t new_nclk = ((3192000000UL * b) / 10000UL);
    uint64_t new_refclk = ((399000000UL * b) / 10000UL);

    for (uint64_t addr = 0; addr < 0x1000000; addr += 8)
    {
        uint64_t v = lv1_peekd(addr);

        if (v == 79800000)
            lv1_poked(addr, value);
        
        if (v == 3192000000UL)
            lv1_poked(addr, new_nclk);

        if (v == 399000000UL)
            lv1_poked(addr, new_refclk);
    }
}

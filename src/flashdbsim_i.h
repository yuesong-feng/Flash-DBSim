/*
 * Flash-DBSim Storage Simulation Environment
 * flashdbsim_i.h - Flash-DBSim calling header file
 * Author: Su.Xuan <sdbchina|mail.ustc.edu.cn>
 *
 * Copyright (c) 2008-2009 KDELab@USTC.
 * Copyright (c) 2022 FENG Yuesong (yuesong-feng@foxmail.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __FLASH_DBSIM_I_H_INCLUDED__
#define __FLASH_DBSIM_I_H_INCLUDED__

#include "guiddef.h"

#ifndef ASSERT
#include <assert.h>
#define ASSERT(booleanExpression) assert(booleanExpression)
#endif  // ASSERT

typedef unsigned char BYTE;

typedef int RV;
typedef int BLOCK_ID;
typedef int PAGE_ID;
typedef void *PVOID; /* void pointer */

typedef int LBA; /* Logical Block Address */

typedef int ID_MODULE;
typedef ID_MODULE IDM_VFD, IDM_MTD, IDM_FTL;

#ifndef INTERFACE_F
#define INTERFACE_F class /* interface identifier */
#endif

#define INTERFACE INTERFACE_F

/* Flash device type: NAND/NOR */
typedef enum FLASH_TYPE {
  NAND = 0, /* NAND type flash */
  NOR = 1,  /* NOR type flash */
} FLASH_TYPE;

/* IUNKNOWN Interface */
typedef INTERFACE_F IUNKNOWN {
 public:
  virtual RV QueryInterface(const IID & /*iid*/, void ** /*ppv*/) = 0;
  // virtual int AddRef(void) = 0;
  // virtual int Release(void) = 0;
}
IUNKNOWN;  // INTERFACE_F IUNKNOWN

/* Interface IUNKNOWN */
// {669FBC31-F562-4b05-9FD6-B18B1517DF38}
const IID IID_IUNKNOWN = {0x669fbc31,
                          0xf562,
                          0x4b05,
                          {0x9f, 0xd6, 0xb1, 0x8b, 0x15, 0x17, 0xdf, 0x38}};
/* Interface of Virtual Flash Device (VFD) Module */
// {CDF32DDF-02CA-4893-9D13-0FD417234934}
const IID IID_IVFD = {
    0xcdf32ddf, 0x2ca, 0x4893, {0x9d, 0x13, 0xf, 0xd4, 0x17, 0x23, 0x49, 0x34}};
/* Interfaces of I/O counters for VFD module */
// {661617C9-9640-427e-9D69-4670422E9C79}
const IID IID_IVFD_COUNTER = {0x661617c9,
                              0x9640,
                              0x427e,
                              {0x9d, 0x69, 0x46, 0x70, 0x42, 0x2e, 0x9c, 0x79}};
/* Interfaces of I/O latencies for VFD module */
// {C3B4DA4D-221C-44a8-9DB8-B672483FE117}
const IID IID_IVFD_LATENCY = {0xc3b4da4d,
                              0x221c,
                              0x44a8,
                              {0x9d, 0xb8, 0xb6, 0x72, 0x48, 0x3f, 0xe1, 0x17}};
/* Interface of Flash Translate Layer Module */
// {1BAC5EDA-18F5-4234-A73C-6411E8392899}
const IID IID_IFTL = {0x1bac5eda,
                      0x18f5,
                      0x4234,
                      {0xa7, 0x3c, 0x64, 0x11, 0xe8, 0x39, 0x28, 0x99}};

/************************************************************************/
/* The following section defines a list of return codes. They are returned by
 * the functions defined in 'Flash-DBSim'.
 */
#define RV_OK 0x0   /* Operation completed successfully */
#define RV_FAIL 0x1 /* Operation failed */

#define RV_ERROR_ARRAY_OUT_BOUND 0x2 /* Array out of bound */
#define RV_ERROR_INVALID_TYPE 0x3    /* Invalid data type */
#define RV_ERROR_FILE_IO 0x4         /* File I/O Exception */

#define RV_ERROR_FLASH_IO_FAILED 0x1000 /* Flash I/O operations are failed */
#define RV_ERROR_FLASH_BLOCK_BROKEN \
  0x1001 /* One block of flash device has broken */
#define RV_ERROR_FLASH_NO_MEMORY 0x1002 /* No more flash memory */
#define RV_ERROR_FLASH_NOT_DIRTY 0x1003 /* Flash memory has no dirty page */
#define RV_ERROR_FLASH_IO_OVERFLOW \
  0x1004 /* Flash I/O operations are overflow with invalid offset or size */

#define RV_ERROR_INVALID_LBA 0x2000        /* Invalid LBA */
#define RV_ERROR_INVALID_PAGE_STATE 0x2001 /* Invalid page state */

#define RV_ERROR_WRONG_MODULE_ID 0x3000 /* Wrong Flash-DBSim module ID */
#define RV_ERROR_MODULE_INITIALIZE_FAILED \
  0x3001 /* Flash-DBSim module initialization failed */

#define RV_ERROR_UNSUPPORT_OBJECT 0x10000      /* Unsupported object */
#define RV_ERROR_UNSUPPORT_INTERFACE_F 0x12345 /* Unsupported interface */
#define RV_ERROR_UNSUPPORT_INTERFACE RV_ERROR_UNSUPPORT_INTERFACE_F

/************************************************************************/
/* VFD Module                                                           */
/************************************************************************/

/* IDs of VFD modules */
typedef enum /*VFD_ID*/ {
  ID_VFD_NONE,

  ID_NAND_DEVICE_01, /* NandDevice01 module */
  ID_NAND_DEVICE_02, /* NandDevice02 module */
  ID_NAND_DEVICE_03, /* NandDevice03 module */
  ID_NAND_DEVICE_04, /* NandDevice04 module */
} VFD_ID;

/* IDs of FTL modules */
typedef enum /*FTL_ID*/ {
  ID_FTL_NONE,

  ID_FTL_01, /* FTL01 module */
} FTL_ID;

/* Information of VFD Module
 */
typedef struct VFD_INFO {
  IDM_VFD id; /* id of VFD Module, used for FlashDBSim */

  int blockCount; /* the number of blocks in flash device */
  struct {
    int size1; /* the size of each page (data area), in bytes */
    int size2; /* the size of each page (additional area), in bytes */

    operator int(void) { return size1 + size2; }
  } pageSize;
  int pageCountPerBlock; /* the number of pages in each block */
  int eraseLimitation;   /* the erase limitation of each block */
  struct {
    int randomTime; /* time of random read operation (MAX.) */
    int serialTime; /* time of serial access operation (Min.) */
  } readTime;
  int programTime; /* time of page program operation */
  int eraseTime;   /* time of block erase operation */

  /* Constructor */
 public:
  VFD_INFO(void) {
    id = 0;
    blockCount = pageSize.size1 = pageSize.size2 = pageCountPerBlock =
        eraseLimitation = 0;
    readTime.randomTime = readTime.serialTime = programTime = eraseTime = 0;
  }
  VFD_INFO(IDM_VFD _id, int bc, int s1, int s2, int pcpb, int el, int rt,
           int st, int pt, int et) {
    id = _id;
    blockCount = bc;
    pageSize.size1 = s1;
    pageSize.size2 = s2;
    pageCountPerBlock = pcpb;
    eraseLimitation = el;
    readTime.randomTime = rt;
    readTime.serialTime = st;
    programTime = pt;
    eraseTime = et;
  }
} VFD_INFO;

typedef INTERFACE_F IVFD_MODULE : public IUNKNOWN {
 protected:
  VFD_INFO info; /* Module Information */
 public:
  virtual ~IVFD_MODULE() = default;

  /* Attributes */
  VFD_INFO GetModuleInfo(void) { return info; } /* get VFD module information */
  virtual FLASH_TYPE GetFlashType(void) {
    return NAND;
  } /* get virtual flash device type */

  /* Methods */
  virtual RV Initialize(
      const VFD_INFO & /*info*/) = 0; /* Initialize VFD Module */
  virtual RV Release(void) = 0;       /* Release VFD Module */

  virtual RV EraseBlock(BLOCK_ID /*blockID*/) = 0; /* erase specified block */
  virtual RV ReadPage(BLOCK_ID /*blockID*/, PAGE_ID /*pageID*/,
                      BYTE * /*buffer*/, int /*offset*/ = 0,
                      int /*size*/ = 0) = 0; /* read specified page */
  virtual RV WritePage(BLOCK_ID /*blockID*/, PAGE_ID /*pageID*/,
                       const BYTE * /*buffer*/, int /*offset*/ = 0,
                       int /*size*/ = 0) = 0; /* write specified page */
}
IVFD;

typedef INTERFACE_F IVFD_COUNTER : public IUNKNOWN {
 protected:
  int *readCounter;  /* read counter of each PAGE */
  int *writeCounter; /* write counter of each PAGE */
  int *eraseCounter; /* erase counter of each BLOCK */

  /* Constructor */
 protected:
  IVFD_COUNTER(void) { readCounter = writeCounter = eraseCounter = NULL; }
  virtual ~IVFD_COUNTER(void) {
    if (readCounter) {
      delete[] readCounter;
      readCounter = NULL;
    }
    if (writeCounter) {
      delete[] writeCounter;
      writeCounter = NULL;
    }
    if (eraseCounter) {
      delete[] eraseCounter;
      eraseCounter = NULL;
    }
  }

 public:
  /* Attributes */
  virtual int GetReadCount(BLOCK_ID /*blockID*/, PAGE_ID /*pageID*/) = 0;
  virtual int GetWriteCount(BLOCK_ID /*blockID*/, PAGE_ID /*pageID*/) = 0;
  virtual int GetEraseCount(BLOCK_ID /*blockID*/) = 0;

  virtual int GetReadCountTotal(void) = 0;
  virtual int GetWriteCountTotal(void) = 0;
  virtual int GetEraseCountTotal(void) = 0;

  virtual void ResetReadCount(void) = 0;
  virtual void ResetWriteCount(void) = 0;
  virtual void ResetEraseCount(void) = 0;
  virtual void ResetCounter(void) = 0;
}
IVFD_COUNTER;

typedef INTERFACE_F IVFD_LATENCY : public IUNKNOWN {
 protected:
  int readLatencyTotal;  /* total latency of READ operations */
  int writeLatencyTotal; /* total latency of WRITE operations */
  int eraseLatencyTotal; /* total latency of ERASE operations */

  /* Constructor */
 protected:
  IVFD_LATENCY(void) {
    readLatencyTotal = writeLatencyTotal = eraseLatencyTotal = 0;
  }

 public:
  /* Attributes */
  int GetReadLatencyTotal(void) { return readLatencyTotal; }
  int GetWriteLatencyTotal(void) { return writeLatencyTotal; }
  int GetEraseLatencyTotal(void) { return eraseLatencyTotal; }

  void ResetReadLatencyTotal(void) { readLatencyTotal = 0; }
  void ResetWriteLatencyTotal(void) { writeLatencyTotal = 0; }
  void ResetEraseLatencyTotal(void) { eraseLatencyTotal = 0; }
  void ResetLatencyTotal(void) {
    readLatencyTotal = writeLatencyTotal = eraseLatencyTotal = 0;
  }
}
IVFD_LATENCY;

/************************************************************************/
/* FTL Module                                                           */
/************************************************************************/

/* Information of FTL Module
 */
typedef struct FTL_INFO {
  IDM_FTL id; /* id of FTL Module, used for Flash-DBSim */

  int mapListSize;           /* size of LBA-PBA map list */
  int wearLevelingThreshold; /* threshold for wear leveling */

  /* Constructor */
 public:
  FTL_INFO(void) : id(ID_FTL_NONE), mapListSize(0), wearLevelingThreshold(0) {}
  FTL_INFO(IDM_FTL _id, int mls, int wlt) {
    id = _id;
    mapListSize = mls;
    wearLevelingThreshold = wlt;
  }
} FTL_INFO;

typedef INTERFACE_F IFTL_MODULE : public IUNKNOWN {
 protected:
  FTL_INFO info;     /* Module Information */
  IVFD *flashDevice; /* related flash device */

 public:
  virtual ~IFTL_MODULE() = default;
  /* Attributes */
  FTL_INFO &GetModuleInfo(void) {
    return info;
  } /* get FTL module information */
  const IVFD *GetFlashDevice(void) {
    return flashDevice;
  } /* get related flash device */

  /* Methods */
  virtual RV Initialize(
      const FTL_INFO & /*info*/,
      const IVFD * /*device*/) = 0; /* initialize FTL module */
  virtual RV Release(void) = 0;     /* release FTL module */

  virtual int AllocPage(
      int /*count*/, LBA * /*lbas*/) = 0; /* apply to allocate some new pages */
  virtual RV ReleasePage(LBA /*lba*/) = 0; /* release one page */
  virtual RV ReadPage(LBA /*lba*/, BYTE * /*buffer*/, int /*offset*/,
                      size_t /*size*/) = 0; /* read specified page */
  virtual RV WritePage(LBA /*lba*/, const BYTE * /*buffer*/, int /*offset*/,
                       size_t /*size*/) = 0; /* write specified page */
}
IFTL;

RV f_initialize(const VFD_INFO &vfdInfo, const FTL_INFO &ftlInfo);

RV f_release(void);

int f_alloc_page(int count, LBA *lbas);

RV f_release_page(LBA lba);

RV f_read_page(LBA lba, BYTE *buffer, int offset, size_t size);

RV f_write_page(LBA lba, const BYTE *buffer, int offset, size_t size);

const IFTL *f_get_ftl_module(void);

const IVFD *f_get_vfd_module(void);

#endif  //__FLASH_DBSIM_I_H_INCLUDED__

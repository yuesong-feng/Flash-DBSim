#pragma once
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>

#include "src/flashdbsim_i.h"

using namespace std;

#define LBA_IS_INVALID 0;
#define LBA_IS_VALID 1;

typedef map<int, int>::value_type valType;
typedef int PID;

#define DEFBUFSIZE 1536  //��������С

#define FRAMESIZE 2048  //��������֡��С

typedef struct bFrame {
  char field[FRAMESIZE];
} bFrame;

typedef struct NewPage {
  int page_id;
  int frame_id;

 public:
  NewPage() {
    page_id = -1;
    frame_id = -1;
  }
  ~NewPage() {}
} NewPage;

/*�������㷨���࣬���еĻ������㷨���Ӹ���̳�*/
class BMgr {
 public:
  BMgr(void);
  ~BMgr(void);

  virtual void Init() = 0;
  virtual int FixPage(int /*page_id*/) = 0;
  virtual NewPage FixNewPage(int /*LBA*/) = 0;
  virtual int UnFixPage(int /*page_id*/) = 0;
  virtual void ReadFrame(int /*frame_id*/, char* /*buffer*/) = 0;
  virtual void WriteFrame(int /*frame_id*/, const char* /*buffer*/) = 0;
  virtual int WriteDirty(void) = 0;
  virtual double HitRatio(void) = 0;
  virtual void RWInfo() = 0;
  virtual int IsLBAValid(LBA /*lba*/) = 0;
  virtual int LBAToPID(LBA /*lba*/) = 0;
};
#pragma once
#include <map>
#include "src/flashdbsim_i.h"

using namespace std;

#define LBA_IS_INVALID 0;
#define LBA_IS_VALID 1;

typedef map<int, int>::value_type valType;
typedef int PID;

#define DEFBUFSIZE 1536  //缓冲区大小

#define FRAMESIZE 2048  //缓冲区中帧大小

typedef struct bFrame {
  char field[FRAMESIZE];
} bFrame;

typedef struct NewPage {
  int page_id;
  int frame_id;

 public:
  NewPage() : page_id(-1), frame_id(-1) {}
  ~NewPage() {}
} NewPage;

/*缓冲区算法基类，所有的缓冲区算法都从该类继承*/
class BMgr {
 public:
  BMgr() = default;
  virtual ~BMgr() = default;

  virtual void Init() = 0;
  virtual int FixPage(int /*page_id*/) = 0;
  virtual NewPage FixNewPage(int /*LBA*/) = 0;
  virtual int UnFixPage(int /*page_id*/) = 0;
  virtual void ReadFrame(int /*frame_id*/, char * /*buffer*/) = 0;
  virtual void WriteFrame(int /*frame_id*/, const char * /*buffer*/) = 0;
  virtual int WriteDirty(void) = 0;
  virtual double HitRatio(void) = 0;
  virtual void RWInfo() = 0;
  virtual int IsLBAValid(LBA /*lba*/) = 0;
  virtual int LBAToPID(LBA /*lba*/) = 0;
};
#include <iostream>

#include "LRU.h"

using namespace std;

BMgr *bmgr;

int main() {
  /*flash ??????????*/
  VFD_INFO vfdInfo;
  FTL_INFO ftlInfo;

  vfdInfo.id = ID_NAND_DEVICE_03;
  vfdInfo.blockCount = 1024;
  vfdInfo.pageCountPerBlock = 64;
  vfdInfo.pageSize.size1 = 2048;
  vfdInfo.pageSize.size2 = 0;
  vfdInfo.eraseLimitation = 100000;
  vfdInfo.readTime.randomTime = 25;
  vfdInfo.readTime.serialTime = 0;
  vfdInfo.programTime = 200;
  vfdInfo.eraseTime = 1500;

  ftlInfo.id = ID_FTL_01;
  ftlInfo.mapListSize = 65536;
  ftlInfo.wearLevelingThreshold = 4;

  LRU *lru = NULL;
  lru = new LRU();
  bmgr = lru;

  printf("FlashDBSim????????......\n");
  f_initialize(vfdInfo, ftlInfo);
  printf("FlashDBSim????????\n");

  FILE *fp = NULL;
  int RW = -1; /*???????????*/
  int lba = -1;
  ;                    /*Trace???????LBA*/
  int fcount = 0;      /*Trace??????*/
  int readcount = 0;   // trace?????????
  int writecount = 0;  // trace??????????
  int i = 1;
  int frid = -1;
  NewPage np;
  np.frame_id = -1;
  np.page_id = -1;
  char *data = new char[FRAMESIZE];
  memset(data, '0', FRAMESIZE);

  /*??????????????10000???????*/
  for (int i = 0; i < 10001; i++) {
    np = bmgr->FixNewPage(i);
    bmgr->WriteFrame(np.frame_id, data);
  }

  /*?????????????????????*/
  bmgr->WriteDirty();

  /*??????????????*/
  bmgr->Init();

  if ((fp = fopen("trace1000000", "r")) == NULL) {
    printf(" cannot open trace file\n");
    return 0;
  }

  while (fcount < 1000000) {
    fscanf(fp, "%d %d", &lba, &RW);

    if (RW == 0) {
      frid = bmgr->FixPage(bmgr->LBAToPID(lba));
      bmgr->ReadFrame(frid, data);
      readcount++;
    } else {
      /*?????????????????????????*/
      if (bmgr->IsLBAValid(lba)) {
        frid = bmgr->FixPage(bmgr->LBAToPID(lba));
        bmgr->WriteFrame(frid, data);
      } else /*???????????????????????????*/
      {
        np = bmgr->FixNewPage(lba);
        bmgr->WriteFrame(np.frame_id, data);
      }
      writecount++;
    }

    fcount++;
    if (fcount == 20000 * i) {
      printf("%d\n", fcount);
      i++;
    }
  }
  fclose(fp);

  bmgr->WriteDirty();

  printf("trace's read count is:%d\n", readcount);
  printf("trace's write count is:%d\n", writecount);

  bmgr->RWInfo();

  printf("hit ratio is:%f\n", bmgr->HitRatio());
  IVFD *vfd = const_cast<IVFD *>(f_get_vfd_module());
  IVFD_COUNTER *icounter = NULL;
  vfd->QueryInterface(IID_IVFD_COUNTER, (void **)&icounter);
  IVFD_LATENCY *ilatency = NULL;
  vfd->QueryInterface(IID_IVFD_LATENCY, (void **)&ilatency);

  printf("ReadCountTotal is %d\n", icounter->GetReadCountTotal());
  printf("WriteCountTotal is %d\n", icounter->GetWriteCountTotal());
  printf("EraseCountTotal is %d\n", icounter->GetEraseCountTotal());
  printf("totalLatency is %d \n",
         ilatency->GetReadLatencyTotal() + ilatency->GetWriteLatencyTotal() +
             ilatency->GetEraseLatencyTotal() - 10001 * 220);
  printf("Write Latency: %d\n", ilatency->GetWriteLatencyTotal());

  FILE *fr = NULL;
  if ((fr = fopen("result_200000.txt", "a")) == NULL) {
    printf(" cannot open result file\n");
    return 0;
  }
  // fprintf(fr,"Running time is:%d",end-start);
  fprintf(fr, "hit ratio is:        %f\n", bmgr->HitRatio());
  fprintf(fr, "ReadCountTotal is:   %d\n",
          ilatency->GetReadLatencyTotal() / 25);
  fprintf(fr, "WriteCountTotal is:  %d\n",
          (ilatency->GetWriteLatencyTotal() - 10001 * 200) / 200);
  fprintf(fr, "EraseCountTotal is:  %d\n",
          ilatency->GetEraseLatencyTotal() / 1500);
  fprintf(fr, "totalLatency is:     %d\n",
          ilatency->GetReadLatencyTotal() + ilatency->GetWriteLatencyTotal() +
              ilatency->GetEraseLatencyTotal() - 10001 * 220);
  fprintf(fr, "\n");
  fclose(fr);

  printf("FlashDBSim???????......\n");
  delete lru;
  lru = NULL;

  delete[] data;
  f_release();
  printf("FlashDBSim??????\n");
  printf("\n");
  printf(
      "........................................................................"
      "..\n");
  printf("\n");
}

#include "LRU.h"

LRU::LRU() {
  for (int i = 0; i < DEFBUFSIZE; i++) {
    ftop[i] = -1;
    ptob[i] = NULL;
    memset(buf[i].field, '\0', FRAMESIZE);
  }
  lru = NULL;
  mru = lru;
  hit = 0;
  total = 0;
  flashreadcount = 0;
  flashwritecount = 0;
}

LRU::~LRU() {
  //释放lru链表
  LRUElement *p = mru;
  while (p != NULL) {
    mru = mru->LessRecent;
    delete p;
    p = mru;
  }
  lru = mru = NULL;

  //释放控制块节点
  LRUBCB *pb = NULL;
  LRUBCB *head = NULL;
  ;
  for (int i = 0; i < DEFBUFSIZE; i++) {
    head = ptob[i];
    pb = head;
    while (pb != NULL) {
      head = head->next;
      delete pb;
      pb = head;
    }
  }
  pb = head = NULL;
  maplist.clear();
}

void LRU::Init() {
  //释放lru链表
  LRUElement *p = mru;
  while (p != NULL) {
    mru = mru->LessRecent;
    delete p;
    p = mru;
  }
  lru = mru = NULL;

  //释放控制块节点
  LRUBCB *pb = NULL;
  LRUBCB *head = NULL;
  ;
  for (int i = 0; i < DEFBUFSIZE; i++) {
    head = ptob[i];
    pb = head;
    while (pb != NULL) {
      head = head->next;
      delete pb;
      pb = head;
    }
  }
  pb = head = NULL;

  /*初始化缓冲区*/
  for (int i = 0; i < DEFBUFSIZE; i++) {
    ftop[i] = -1;
    ptob[i] = NULL;
    memset(buf[i].field, '\0', FRAMESIZE);
  }
  hit = 0;
  total = 0;
  flashreadcount = 0;
  flashwritecount = 0;
}

int LRU::FixPage(int page_id) {
  ASSERT(page_id >= 0);

  int frid = -1;
  int rv = -1;
  int hk = hash(page_id);
  LRUBCB *pb = NULL;
  pb = PageToLRUBCB(page_id);

  /*读计数加1*/
  total++;

  /*若该页在缓冲区中已存在*/
  if (pb != NULL) {
    frid = pb->frame_id;
    AdjustLRUList(frid);

    /*命中次数加1*/
    hit++;

    return frid;
  } else  // 若该页不在缓冲区中
  {
    frid = SelectVictim();

    /*正常情形下，此时应该从二级存储器读入需要加载的数据*/
    rv = f_read_page(page_id, (BYTE *)(buf[frid].field), 0, FRAMESIZE);
    if (rv == RV_ERROR_INVALID_PAGE_STATE) {
      printf("page readed is invalid\n");
    }
    if (rv == RV_ERROR_FLASH_BLOCK_BROKEN) {
      printf("the block contained this page is broken \n");
    }

    flashreadcount++;

    ftop[frid] = page_id;
    pb = ptob[hk];
    /*将新的LRUBCB链接到桶的尾部*/
    if (pb != NULL) {
      while (pb->next != NULL) {
        pb = pb->next;
      }
      pb->next = new LRUBCB();
      pb = pb->next;
    } else  //桶为空
    {
      pb = new LRUBCB();
      ptob[hk] = pb;
    }
    pb->dirty = 0;
    pb->frame_id = frid;
    pb->page_id = page_id;
    pb->next = NULL;
    InsertLRUEle(frid);
    return frid;
  }
}

NewPage LRU::FixNewPage(LBA lba) {
  NewPage np;
  np.frame_id = -1;
  np.page_id = -1;
  int pid = -1;
  int frid = -1;
  LRUBCB *pb = NULL;

  /* 申请分配新数据页 */
  f_alloc_page(1, &pid);
  if (pid == -1) printf("there is no free page in the flash memory");
  ASSERT(pid >= 0);

  /*注册映射项*/
  RegistEntry(lba, pid);

  /* 在缓冲区中寻找空闲块，用于存储新数据页中的数据*/
  frid = SelectVictim();
  ftop[frid] = pid;
  pb = ptob[hash(pid)];
  /*将新的LRUBCB链接到桶的尾部*/
  if (pb != NULL) {
    while (pb->next != NULL) {
      pb = pb->next;
    }
    pb->next = new LRUBCB();
    pb = pb->next;
  } else  //桶为空
  {
    pb = new LRUBCB();
    ptob[hash(pid)] = pb;
  }
  pb->dirty = 0;
  pb->frame_id = frid;
  pb->page_id = pid;
  pb->next = NULL;
  InsertLRUEle(frid);
  np.frame_id = frid;
  np.page_id = pid;
  return np;
}

int LRU::UnFixPage(int page_id) {
  ASSERT(page_id >= 0);
  LRUBCB *pb = NULL;
  pb = PageToLRUBCB(page_id);
  int rv = -1;

  if (pb->dirty == 1) {
    pb->dirty = 0;
  }

  rv = f_release_page(page_id);
  if (rv == RV_ERROR_INVALID_LBA) {
    printf("Invalid  LBA\n");
    return rv;
  }
  return 0;
}

void LRU::ReadFrame(int frame_id, char *buffer) {
  ASSERT(frame_id >= 0);
  ASSERT(buffer != NULL);

  memcpy(buffer, buf[frame_id].field, FRAMESIZE);
  return;
}

void LRU::WriteFrame(int frame_id, const char *buffer) {
  ASSERT(frame_id >= 0);
  ASSERT(buffer != NULL);

  memcpy(buf[frame_id].field, buffer, FRAMESIZE);
  SetDirty(frame_id);
  return;
}

int LRU::WriteDirty() {
  LRUBCB *pb = NULL;
  int rv = -1;
  for (int i = 0; i < DEFBUFSIZE; i++) {
    pb = ptob[i];
    while (pb != NULL) {
      if (pb->dirty == 1) {
        //程序结束时，我们应该将脏页写回二级存储器
        rv = f_write_page(pb->page_id, (BYTE *)(buf[pb->frame_id].field), 0,
                          FRAMESIZE);
        if (rv == RV_ERROR_FLASH_NO_MEMORY) {
          printf("no more flash memory \n");
          return rv;
        }

        flashwritecount++;
      }
      pb = pb->next;
    }
  }
  return 0;
}

double LRU::HitRatio() { return ((double)(hit) / (double)(total)); }

void LRU::RWInfo() {
  printf("hit is: %d\n", hit);
  printf("total is: %d\n", total);
  printf("flash read count is: %d\n", flashreadcount);
  printf("flash write count is: %d\n", flashwritecount);
  return;
}

int LRU::IsLBAValid(LBA lba) {
  ASSERT(lba >= 0);
  if (maplist.count(lba)) {
    return LBA_IS_VALID;
  } else {
    return LBA_IS_INVALID;
  }
}

int LRU::LBAToPID(LBA lba) {
  ASSERT(lba >= 0);
  return maplist[lba];
}

void LRU::SetDirty(int frame_id) {
  ASSERT(frame_id >= 0);
  int pid = -1;
  LRUBCB *pb = NULL;
  pid = ftop[frame_id];
  pb = PageToLRUBCB(pid);
  if (pb->dirty == 0) {
    pb->dirty = 1;
  }
  return;
}

int LRU::hash(int page_id) {
  ASSERT(page_id >= 0);

  int hk = -1;
  hk = page_id % DEFBUFSIZE;
  return hk;
}

LRUBCB *LRU::PageToLRUBCB(int page_id) {
  ASSERT(page_id >= 0);

  LRUBCB *pb = NULL;
  int hk = -1;
  hk = hash(page_id);
  pb = ptob[hk];
  while (pb != NULL && pb->page_id != page_id) {
    pb = pb->next;
  }
  return pb;
}

void LRU::RemoveLRUBCB(LRUBCB *pb) {
  ASSERT(pb != NULL);

  LRUBCB *head = NULL;
  head = ptob[hash(pb->page_id)];

  /* 若该LRUBCB正好在桶的首部 */
  if (pb == head) {
    /* 如果桶的长度大于1，则需要维护桶的其它部分*/
    if (head->next != NULL) {
      head = head->next;
      ptob[hash(pb->page_id)] = head;
      ftop[pb->frame_id] = -1;
      memset(&(buf[pb->frame_id].field), '\0', FRAMESIZE);
      delete pb;
      pb = NULL;
      head = NULL;
    } else  //若桶中只有一个元素
    {
      ptob[hash(pb->page_id)] = NULL;
      ftop[pb->frame_id] = -1;
      memset(&(buf[pb->frame_id].field), '\0', FRAMESIZE);
      delete pb;
      pb = NULL;
    }
  } else  //在桶内需找LRUBCB的正确位置
  {
    while (head->next != pb) {
      head = head->next;
    }
    head->next = pb->next;
    ftop[pb->frame_id] = -1;
    memset(&(buf[pb->frame_id].field), '\0', FRAMESIZE);
    delete pb;
    pb = NULL;
    head = NULL;
  }
  return;
}

/* 在LRU置换策略中，每次只需删除lru指向的元素，这里做一般化处理*/
void LRU::RemoveLRUEle(int frame_id) {
  ASSERT(frame_id >= 0);
  LRUElement *elem = NULL;
  elem = lru;
  if (elem == NULL) {
    return;
  }

  while ((elem != NULL) && (elem->frame_id != frame_id)) {
    elem = elem->MoreRecent;
  }

  if (elem == NULL) {
    return;  //找不到相应的元素
  } else {
    if (elem == lru)  // lru指向要删除的元素
    {
      if (elem == mru)  // mru也指向要删除的元素
      {
        lru = mru = NULL;
        delete elem;
      } else {
        lru = lru->MoreRecent;
        lru->LessRecent = NULL;
        delete elem;
      }
    } else  //在lru策略中能删除除链尾以外的元素，只出现在多线程访问同一元素时（数据锁）
    {
      if (elem == mru)  // mru指向要删除的元素
      {
        mru = mru->LessRecent;
        mru->MoreRecent = NULL;
        delete elem;
      } else  //删除除链头和链尾的元素
      {
        elem->MoreRecent->LessRecent = elem->LessRecent;
        elem->LessRecent->MoreRecent = elem->MoreRecent;
        delete elem;
      }
    }  // end elem == lru
    return;
  }  // end elem == NULL
}

void LRU::InsertLRUEle(int frame_id) {
  ASSERT(frame_id >= 0);

  LRUElement *elem = NULL;
  elem = new LRUElement();
  elem->frame_id = frame_id;
  elem->LessRecent = NULL;
  elem->MoreRecent = NULL;
  /* 若链表为空 */
  if (mru == NULL) {
    mru = elem;
    lru = mru;
  } else {
    elem->LessRecent = mru;
    mru->MoreRecent = elem;
    mru = elem;
  }
  return;
}

void LRU::AdjustLRUList(int frame_id) {
  ASSERT(frame_id >= 0);
  LRUElement *elem = NULL;
  elem = mru;

  /* 若链表为空*/
  if (elem == NULL) {
    return;
  }

  while ((elem != NULL) && (elem->frame_id != frame_id)) {
    elem = elem->LessRecent;
  }

  /*若没找到*/
  if (elem == NULL) {
    return;
  } else {
    if (elem == mru)  //访问的元素在链首，直接返回
    {
      return;
    } else  //访问的元素在链尾，将其移到链首
    {
      if (elem == lru)  //若访问的元素为链尾元素
      {
        lru = lru->MoreRecent;
        lru->LessRecent = NULL;
      } else  //访问除链首和链尾之外的元素
      {
        elem->LessRecent->MoreRecent = elem->MoreRecent;
        elem->MoreRecent->LessRecent = elem->LessRecent;
      }
      elem->LessRecent = mru;
      mru->MoreRecent = elem;
      elem->MoreRecent = NULL;
      mru = elem;
      return;
    }  // end elem == mru
  }    // end elem == NULL
}

int LRU::SelectVictim() {
  LRUBCB *pb = NULL;
  LRUElement *elem = NULL;
  int frid = -1;
  int rv = -1;

  /* 若干净页链表和脏页链表都为空，则选择缓冲区中的第一块 */
  if (lru == NULL) {
    return 0;
  }

  /* 从缓冲区中找空闲块 */
  for (int i = 0; i < DEFBUFSIZE; i++) {
    if (ftop[i] == -1) {
      return i;
    }
  }

  /* 若缓冲区已无空闲页，则从中选择置换页（执行置换策略）*/
  elem = lru;
  lru = lru->MoreRecent;
  lru->LessRecent = NULL;
  frid = elem->frame_id;
  pb = PageToLRUBCB(ftop[frid]);

  if (pb->dirty == 1) {
    //正常情况下，应该将数据写回二级存储器，这里只计数
    rv = f_write_page(pb->page_id, (BYTE *)(buf[pb->frame_id].field), 0,
                      FRAMESIZE);
    if (rv == RV_ERROR_FLASH_NO_MEMORY) printf("no more flash memory \n");

    flashwritecount++;
  }

  /* 将置换帧的元信息和链表信息删除 */
  RemoveLRUEle(frid);
  RemoveLRUBCB(pb);
  return frid;
}

void LRU::RegistEntry(LBA lba, PID page_id) {
  ASSERT(lba >= 0);
  ASSERT(page_id >= 0);
  maplist.insert(valType(lba, page_id));
}
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
  //�ͷ�lru����
  LRUElement* p = mru;
  while (p != NULL) {
    mru = mru->LessRecent;
    delete p;
    p = mru;
  }
  lru = mru = NULL;

  //�ͷſ��ƿ�ڵ�
  LRUBCB* pb = NULL;
  LRUBCB* head = NULL;
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
  //�ͷ�lru����
  LRUElement* p = mru;
  while (p != NULL) {
    mru = mru->LessRecent;
    delete p;
    p = mru;
  }
  lru = mru = NULL;

  //�ͷſ��ƿ�ڵ�
  LRUBCB* pb = NULL;
  LRUBCB* head = NULL;
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

  /*��ʼ��������*/
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
  LRUBCB* pb = NULL;
  pb = PageToLRUBCB(page_id);

  /*��������1*/
  total++;

  /*����ҳ�ڻ��������Ѵ���*/
  if (pb != NULL) {
    frid = pb->frame_id;
    AdjustLRUList(frid);

    /*���д�����1*/
    hit++;

    return frid;
  } else  // ����ҳ���ڻ�������
  {
    frid = SelectVictim();

    /*���������£���ʱӦ�ôӶ����洢��������Ҫ���ص�����*/
    rv = f_read_page(page_id, (BYTE*)(buf[frid].field), 0, FRAMESIZE);
    if (rv == RV_ERROR_INVALID_PAGE_STATE) {
      printf("page readed is invalid\n");
    }
    if (rv == RV_ERROR_FLASH_BLOCK_BROKEN) {
      printf("the block contained this page is broken \n");
    }

    flashreadcount++;

    ftop[frid] = page_id;
    pb = ptob[hk];
    /*���µ�LRUBCB���ӵ�Ͱ��β��*/
    if (pb != NULL) {
      while (pb->next != NULL) {
        pb = pb->next;
      }
      pb->next = new LRUBCB();
      pb = pb->next;
    } else  //ͰΪ��
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
  LRUBCB* pb = NULL;

  /* �������������ҳ */
  f_alloc_page(1, &pid);
  if (pid == -1) printf("there is no free page in the flash memory");
  ASSERT(pid >= 0);

  /*ע��ӳ����*/
  RegistEntry(lba, pid);

  /* �ڻ�������Ѱ�ҿ��п飬���ڴ洢������ҳ�е�����*/
  frid = SelectVictim();
  ftop[frid] = pid;
  pb = ptob[hash(pid)];
  /*���µ�LRUBCB���ӵ�Ͱ��β��*/
  if (pb != NULL) {
    while (pb->next != NULL) {
      pb = pb->next;
    }
    pb->next = new LRUBCB();
    pb = pb->next;
  } else  //ͰΪ��
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
  LRUBCB* pb = NULL;
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

void LRU::ReadFrame(int frame_id, char* buffer) {
  ASSERT(frame_id >= 0);
  ASSERT(buffer != NULL);

  memcpy(buffer, buf[frame_id].field, FRAMESIZE);
  return;
}

void LRU::WriteFrame(int frame_id, const char* buffer) {
  ASSERT(frame_id >= 0);
  ASSERT(buffer != NULL);

  memcpy(buf[frame_id].field, buffer, FRAMESIZE);
  SetDirty(frame_id);
  return;
}

int LRU::WriteDirty() {
  LRUBCB* pb = NULL;
  int rv = -1;
  for (int i = 0; i < DEFBUFSIZE; i++) {
    pb = ptob[i];
    while (pb != NULL) {
      if (pb->dirty == 1) {
        //�������ʱ������Ӧ�ý���ҳд�ض����洢��
        rv = f_write_page(pb->page_id, (BYTE*)(buf[pb->frame_id].field), 0,
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
  LRUBCB* pb = NULL;
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

LRUBCB* LRU::PageToLRUBCB(int page_id) {
  ASSERT(page_id >= 0);

  LRUBCB* pb = NULL;
  int hk = -1;
  hk = hash(page_id);
  pb = ptob[hk];
  while (pb != NULL && pb->page_id != page_id) {
    pb = pb->next;
  }
  return pb;
}

void LRU::RemoveLRUBCB(LRUBCB* pb) {
  ASSERT(pb != NULL);

  LRUBCB* head = NULL;
  head = ptob[hash(pb->page_id)];

  /* ����LRUBCB������Ͱ���ײ� */
  if (pb == head) {
    /* ���Ͱ�ĳ��ȴ���1������Ҫά��Ͱ����������*/
    if (head->next != NULL) {
      head = head->next;
      ptob[hash(pb->page_id)] = head;
      ftop[pb->frame_id] = -1;
      memset(&(buf[pb->frame_id].field), '\0', FRAMESIZE);
      delete pb;
      pb = NULL;
      head = NULL;
    } else  //��Ͱ��ֻ��һ��Ԫ��
    {
      ptob[hash(pb->page_id)] = NULL;
      ftop[pb->frame_id] = -1;
      memset(&(buf[pb->frame_id].field), '\0', FRAMESIZE);
      delete pb;
      pb = NULL;
    }
  } else  //��Ͱ������LRUBCB����ȷλ��
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

/* ��LRU�û������У�ÿ��ֻ��ɾ��lruָ���Ԫ�أ�������һ�㻯����*/
void LRU::RemoveLRUEle(int frame_id) {
  ASSERT(frame_id >= 0);
  LRUElement* elem = NULL;
  elem = lru;
  if (elem == NULL) {
    return;
  }

  while ((elem != NULL) && (elem->frame_id != frame_id)) {
    elem = elem->MoreRecent;
  }

  if (elem == NULL) {
    return;  //�Ҳ�����Ӧ��Ԫ��
  } else {
    if (elem == lru)  // lruָ��Ҫɾ����Ԫ��
    {
      if (elem == mru)  // mruҲָ��Ҫɾ����Ԫ��
      {
        lru = mru = NULL;
        delete elem;
      } else {
        lru = lru->MoreRecent;
        lru->LessRecent = NULL;
        delete elem;
      }
    } else  //��lru��������ɾ������β�����Ԫ�أ�ֻ�����ڶ��̷߳���ͬһԪ��ʱ����������
    {
      if (elem == mru)  // mruָ��Ҫɾ����Ԫ��
      {
        mru = mru->LessRecent;
        mru->MoreRecent = NULL;
        delete elem;
      } else  //ɾ������ͷ����β��Ԫ��
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

  LRUElement* elem = NULL;
  elem = new LRUElement();
  elem->frame_id = frame_id;
  elem->LessRecent = NULL;
  elem->MoreRecent = NULL;
  /* ������Ϊ�� */
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
  LRUElement* elem = NULL;
  elem = mru;

  /* ������Ϊ��*/
  if (elem == NULL) {
    return;
  }

  while ((elem != NULL) && (elem->frame_id != frame_id)) {
    elem = elem->LessRecent;
  }

  /*��û�ҵ�*/
  if (elem == NULL) {
    return;
  } else {
    if (elem == mru)  //���ʵ�Ԫ�������ף�ֱ�ӷ���
    {
      return;
    } else  //���ʵ�Ԫ������β�������Ƶ�����
    {
      if (elem == lru)  //�����ʵ�Ԫ��Ϊ��βԪ��
      {
        lru = lru->MoreRecent;
        lru->LessRecent = NULL;
      } else  //���ʳ����׺���β֮���Ԫ��
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
  LRUBCB* pb = NULL;
  LRUElement* elem = NULL;
  int frid = -1;
  int rv = -1;

  /* ���ɾ�ҳ�������ҳ����Ϊ�գ���ѡ�񻺳����еĵ�һ�� */
  if (lru == NULL) {
    return 0;
  }

  /* �ӻ��������ҿ��п� */
  for (int i = 0; i < DEFBUFSIZE; i++) {
    if (ftop[i] == -1) {
      return i;
    }
  }

  /* �����������޿���ҳ�������ѡ���û�ҳ��ִ���û����ԣ�*/
  elem = lru;
  lru = lru->MoreRecent;
  lru->LessRecent = NULL;
  frid = elem->frame_id;
  pb = PageToLRUBCB(ftop[frid]);

  if (pb->dirty == 1) {
    //��������£�Ӧ�ý�����д�ض����洢��������ֻ����
    rv = f_write_page(pb->page_id, (BYTE*)(buf[pb->frame_id].field), 0,
                      FRAMESIZE);
    if (rv == RV_ERROR_FLASH_NO_MEMORY) printf("no more flash memory \n");

    flashwritecount++;
  }

  /* ���û�֡��Ԫ��Ϣ��������Ϣɾ�� */
  RemoveLRUEle(frid);
  RemoveLRUBCB(pb);
  return frid;
}

void LRU::RegistEntry(LBA lba, PID page_id) {
  ASSERT(lba >= 0);
  ASSERT(page_id >= 0);
  maplist.insert(valType(lba, page_id));
}
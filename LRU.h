#pragma once
#include "BufferAlogrithm.h"


typedef struct LRUBCB {
public:
	int page_id;
	int frame_id;
	int dirty;
	LRUBCB* next;
	
public:
	LRUBCB() {page_id = -1; frame_id = -1; dirty = -1; next = NULL;}
	~LRUBCB() {}
} LRUBCB;


typedef  struct LRUElement {
public:
	int frame_id;
	LRUElement* LessRecent;
	LRUElement* MoreRecent;

public:
	LRUElement() {frame_id = -1;LessRecent = NULL; MoreRecent = NULL;}
	~LRUElement() {}
} LRUElement;


class LRU : public BMgr
{
public:
	LRU();
	~LRU();
	void Init();
	int FixPage(int page_id);
    NewPage FixNewPage(LBA lba);
	int UnFixPage(int page_id);
	void ReadFrame(int frame_id,char* buffer);
	void WriteFrame(int frame_id,const char*buffer);
	int WriteDirty(void);
	double HitRatio(void);
	void RWInfo();
	int IsLBAValid(LBA lba);
	int LBAToPID(LBA lba);

private:
	int hash(int page_id);
	LRUBCB* PageToLRUBCB(int page_id);
	void RemoveLRUBCB(LRUBCB* pb);
	void InsertLRUEle(int frame_id);
	void RemoveLRUEle(int frame_id);
	void AdjustLRUList(int frame_id);
	void SetDirty(int frame_id);
	int SelectVictim();
	void RegistEntry(LBA lba, PID page_id);

private:
	int ftop[DEFBUFSIZE];
	LRUBCB* ptob[DEFBUFSIZE];
	bFrame buf[DEFBUFSIZE];
	map<LBA,PID> maplist;
	LRUElement* lru;
	LRUElement* mru;
	int hit;
	int total;
	int flashreadcount;
	int flashwritecount;
};
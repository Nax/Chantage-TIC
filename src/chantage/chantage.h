#ifndef CHANTAGE_H
#define CHANTAGE_H

#include <stdint.h>

/* Hooks */
void* BaseRelPtr(uint32_t off);

/* Init */
void ChantageInit(void);
void WriteProtected(void* dst, const void* src, size_t size);
void WriteProtected64(void* dst, uint64_t val);
void WriteProtected32(void* dst, uint32_t val);
void WriteProtected16(void* dst, uint16_t val);
void WriteProtected8(void* dst, uint8_t val);
void WriteProtectedRel(uint32_t off, const void* src, size_t size);
void WriteProtectedRel64(uint32_t off, uint64_t val);
void WriteProtectedRel32(uint32_t off, uint32_t val);
void WriteProtectedRel16(uint32_t off, uint16_t val);
void WriteProtectedRel8(uint32_t off, uint8_t val);

void HookFunction(void* target, void* hook);
void HookFunctionRel(uint32_t off, void* hook);

typedef struct
{
    uint8_t palette;
    uint8_t gfx;
    uint8_t level;
    uint8_t flags;
    uint8_t unk0;
    uint8_t type;
    uint8_t unk1;
    uint8_t attrId;
    uint16_t price;
    uint8_t shop;
    uint8_t unk2;
} ItemData;

/* Init */
void Init_Items(void);

#endif

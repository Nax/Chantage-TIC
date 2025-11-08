#ifndef CHANTAGE_HOOKS_H
#define CHANTAGE_HOOKS_H

#include <chantage/types.h>

void* BaseRelPtr(uint32_t off);

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

void Hook_CallTrampoline32Rel(uint32_t off, void* hook);

#endif

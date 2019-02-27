#ifndef E8080MEMORY_H
#define E8080MEMORY_H

typedef struct Mem Mem;

uint8_t *registerBank(uint32_t base, uint32_t size, uint8_t *buf, int ro);
void registerMirror(uint32_t base, uint32_t size, uint32_t to);

void writeByte (uint8_t value, uint16_t offset);
uint8_t readByte (uint16_t offset);
void stackPush (uint16_t value);
uint16_t stackPop();

#endif

#ifndef GPT_H
#define GPT_H

#include "util.h"
#include <stdint.h>

#include "btstdbeg.h"

BOOT_STRUCT(boot_GPTPartition) {
  unsigned char typeGUID[16];
  unsigned char partGUID[16];
  uint64_t firstLBA;
  uint64_t lastLBA;
  uint64_t attributes;
};

#include "btstdend.h"

#ifdef __cplusplus
namespace boot {
using GPTPartition = ::boot_GPTPartition;
}
#endif

#endif // GPT_H

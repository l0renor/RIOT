/*
 * Copyright (C) 2019 Koen Zandberg
 *               2019 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @ingroup     sys_suit
 * @brief       SUIT policy definition
 *
 * @{
 *
 * @brief       SUIT policy definitions
 * @author      Koen Zandberg <koen@bergzand.net>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 */

#ifndef SUIT_V4_POLICY_H
#define SUIT_V4_POLICY_H

#include <stddef.h>
#include <stdint.h>

#include "uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUIT_VALIDATED_AUTH         0x1
#define SUIT_VALIDATED_VERSION      0x2
#define SUIT_VALIDATED_SEQ_NR       0x4
#define SUIT_VALIDATED_VENDOR       0x8
#define SUIT_VALIDATED_CLASS        0x10
#define SUIT_VALIDATED_DEVICE       0x20

#define SUIT_DEFAULT_POLICY \
          (SUIT_VALIDATED_VERSION | SUIT_VALIDATED_SEQ_NR | SUIT_VALIDATED_VENDOR | SUIT_VALIDATED_CLASS)

#ifdef __cplusplus
}
#endif

#endif /* SUIT_V4_POLICY_H */
/** @} */
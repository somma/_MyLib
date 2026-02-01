/**
 * @file    machine_id.h
 * @brief	This module generates an almost unique hardware id.
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    07.22.2022 15:13 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#pragma once
#include "BaseWindowsHeader.h"
#include <intrin.h>

bool generate_machine_id(_Out_ std::string& machine_id);



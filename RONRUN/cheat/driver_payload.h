#pragma once
// ============================================================================
//  Embedded Driver Payload (Optional)
//  Allows bundling a precompiled kernel driver directly into the single executable.
//  If g_driver_payload_size == 0, the engine uses Usermode Direct mode seamlessly.
// ============================================================================

#include <cstdint>
#include <cstddef>

inline const uint8_t g_driver_payload[] = {};
inline const size_t g_driver_payload_size = 0;

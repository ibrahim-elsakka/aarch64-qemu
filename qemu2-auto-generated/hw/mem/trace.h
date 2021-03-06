/* This file is autogenerated by tracetool, do not edit. */

#ifndef TRACE_HW_MEM_GENERATED_TRACERS_H
#define TRACE_HW_MEM_GENERATED_TRACERS_H

#include "qemu-common.h"
#include "trace/control.h"

extern TraceEvent _TRACE_MHP_PC_DIMM_ASSIGNED_SLOT_EVENT;
extern TraceEvent _TRACE_MHP_PC_DIMM_ASSIGNED_ADDRESS_EVENT;
extern uint16_t _TRACE_MHP_PC_DIMM_ASSIGNED_SLOT_DSTATE;
extern uint16_t _TRACE_MHP_PC_DIMM_ASSIGNED_ADDRESS_DSTATE;
#define TRACE_MHP_PC_DIMM_ASSIGNED_SLOT_ENABLED 1
#define TRACE_MHP_PC_DIMM_ASSIGNED_ADDRESS_ENABLED 1

#define TRACE_MHP_PC_DIMM_ASSIGNED_SLOT_BACKEND_DSTATE() ( \
    false)

static inline void _nocheck__trace_mhp_pc_dimm_assigned_slot(int slot)
{
}

static inline void trace_mhp_pc_dimm_assigned_slot(int slot)
{
    if (true) {
        _nocheck__trace_mhp_pc_dimm_assigned_slot(slot);
    }
}

#define TRACE_MHP_PC_DIMM_ASSIGNED_ADDRESS_BACKEND_DSTATE() ( \
    false)

static inline void _nocheck__trace_mhp_pc_dimm_assigned_address(uint64_t addr)
{
}

static inline void trace_mhp_pc_dimm_assigned_address(uint64_t addr)
{
    if (true) {
        _nocheck__trace_mhp_pc_dimm_assigned_address(addr);
    }
}
#endif /* TRACE_HW_MEM_GENERATED_TRACERS_H */

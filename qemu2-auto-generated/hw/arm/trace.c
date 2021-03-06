/* This file is autogenerated by tracetool, do not edit. */

#include "qemu/osdep.h"
#include "trace.h"

uint16_t _TRACE_VIRT_ACPI_SETUP_DSTATE;
TraceEvent _TRACE_VIRT_ACPI_SETUP_EVENT = {
    .id = 0,
    .vcpu_id = TRACE_VCPU_EVENT_NONE,
    .name = "virt_acpi_setup",
    .sstate = TRACE_VIRT_ACPI_SETUP_ENABLED,
    .dstate = &_TRACE_VIRT_ACPI_SETUP_DSTATE 
};
TraceEvent *hw_arm_trace_events[] = {
    &_TRACE_VIRT_ACPI_SETUP_EVENT,
  NULL,
};

static void trace_hw_arm_register_events(void)
{
    trace_event_register_group(hw_arm_trace_events);
}
trace_init(trace_hw_arm_register_events)

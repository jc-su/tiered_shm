#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

SEC("tracepoint/syscalls/sys_enter_mmap")
int mmap_monitor(struct trace_event_raw_sys_enter *ctx) {
    // handle mmap syscall event
    return 0;
}

char _license[] SEC("license") = "GPL";

#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

struct bpf_map_def SEC("maps") events = {
    .type = BPF_MAP_TYPE_PERF_EVENT_ARRAY,
    .key_size = sizeof(int),
    .value_size = sizeof(int),
    .max_entries = 0,
};

SEC("tracepoint/syscalls/sys_enter_mmap")
int mmap_monitor(void *ctx) {
    int key = 0;
    long value = 1;
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &value, sizeof(value));
    return 0;
}

char _license[] SEC("license") = "GPL";

#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

struct bpf_map_def SEC("maps") events = {
    .type = BPF_MAP_TYPE_PERF_EVENT_ARRAY,
    .key_size = sizeof(int),
    .value_size = sizeof(int),
    .max_entries = 0,
};

SEC("tracepoint/syscalls/sys_enter_mmap")
int mmap_monitor(struct trace_event_raw_sys_enter *ctx) {
    // handle mmap syscall event
    // For example, you might want to send an event to user-space:
    int key = 0;
    long value = 1; // Placeholder value
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &value, sizeof(value));
    return 0;
}

char _license[] SEC("license") = "GPL";
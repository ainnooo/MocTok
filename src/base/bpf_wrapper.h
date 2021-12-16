#ifndef BPF_WRAPPER_H_
#define BPF_WRAPPER_H_

#include <string>

#include <bpf/bpf.h>

namespace bpf {

// Helper function to get bpf_map_info from specified |fd|.
// Returns 0 in success.
int GetMapInfoByFd(int fd, struct bpf_map_info* info);

// Helper funciton to get fd of the bpf object in |path|.
// Returns fd.
int GetPinnedObjFd(const std::string& path);

// Helper function to get map value from user space.
// Returns 0 in success.
int MapLookupElem(int map_fd, void* key, void* value);

}  // namespace bpf

#endif  // BPF_WRAPPER_H_

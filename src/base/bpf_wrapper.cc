#include "bpf_wrapper.h"

#include <string>

#include <bpf/bpf.h>

namespace bpf {

int GetMapInfoByFd(int fd, struct bpf_map_info* info) {
  __u32 info_len = sizeof(*info);
  // returns err
  return bpf_obj_get_info_by_fd(fd, info, &info_len);
}

int GetPinnedObjFd(const std::string& path) {
  return bpf_obj_get(path.c_str());
}

int MapLookupElem(int map_fd, void* key, void* value) {
  return bpf_map_lookup_elem(map_fd, key, value);
}

}  // namespace bpf

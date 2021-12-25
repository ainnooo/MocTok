#include "loader.h"

#include <cassert>
#include <csignal>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <stdio.h>
#include <unistd.h>

#include <linux/if_link.h>
#include <net/if.h>

#include "base/bpf_wrapper.h"
#include "base/define/define.h"
#include "base/logger.h"

Loader::Loader(const Mode mode,
               const unsigned int xdp_flags,
               const unsigned int ifindex,
               const std::string& ifname,
               const std::string& bpf_filepath,
               const std::string& progsec,
               const LogLevel& level)
    : mode_(mode),
      xdp_flags_(xdp_flags),
      ifindex_(ifindex),
      ifname_(ifname),
      bpf_filepath_(bpf_filepath),
      progsec_(progsec),
      logger_(Logger(level)) {}

Loader::Loader(const Mode mode,
               const unsigned int xdp_flags,
               const unsigned int ifindex,
               const std::string& ifname)
    : mode_(mode),
      xdp_flags_(xdp_flags),
      ifindex_(ifindex),
      ifname_(ifname),
      logger_(Logger(LogLevel::Info)) {}

void Loader::Start() {
  switch (mode_) {
    case Mode::Attach:
      if (AttachBpf() != kSuccess) {
        logger_.Error("Failed: suspend loading program,");
        return;
      }
      PinMaps();
      break;
    case Mode::Detach:
      DetachBpf();
      break;
    default:
      assert(false);
      break;
  }
  return;
}

void Loader::DetachBpf() {
  // SetBpf() unloads BPF program when fd is -1.
  prog_fd_ = -1;
  int err = SetBpf();
  if (err == kSuccess) {
    logger_.Info("Success: Bpf program is unloaded from interface.");
  } else {
    logger_.Error("Failed: Bpf program cannot be unloaded from interface.");
  }
}

int Loader::AttachBpf() {
  // Load the BPF-ELF file.
  bpf_wrapper_ = std::make_unique<BpfWrapper>(bpf_filepath_);
  int err = bpf_wrapper_->Load();
  if (err) {
    std::cerr << "ERR: cannot load BPF-OBJ file at " << bpf_filepath_
              << std::endl;
    return kError;
  }

  // Find fd of the specified prog section.
  prog_fd_ = bpf_wrapper_->GetSectionFd(progsec_);
  if (prog_fd_ <= 0) {
    std::cerr << "ERR: Failed to get bpf program fd." << std::endl;
    return kError;
  }

  // Set fd to the interface.
  err = SetBpf();
  if (err == kSuccess) {
    logger_.Info("Success: Bpf program is loaded to interface ");
  } else {
    logger_.Error("Failed: Bpf program cannot be loaded to interface ");
  }

  return err;
}

void Loader::PinMaps() {
  assert(bpf_wrapper_);

  std::string pin_dir = "/sys/fs/bpf/" + ifname_;
  int err;
  // Unpin maps in advance.
  if (access(pin_dir.c_str(), F_OK) != -1) {
    err = bpf_wrapper_->UnpinMaps(pin_dir);
    if (err) {
      std::cerr << "ERR: Unpinning maps." << std::endl;
      return;
    }
    std::cout << "Unpinned maps!!!" << std::endl;
  }

  // Pin maps.
  err = bpf_wrapper_->PinMaps(pin_dir);
  if (err) {
    std::cerr << "ERR: Pinning maps at " << pin_dir << std::endl;
    return;
  }
  std::clog << "Success: Pinning maps at " << pin_dir << std::endl;
}

int Loader::SetBpf() {
  // set |prog_fd_| to the interface.
  int err = BpfWrapper::SetFdToInterface(ifindex_, prog_fd_, xdp_flags_);
  if (err < 0) {
    std::cerr << "ERR: Set xdp fd to " << ifname_ << " failed." << std::endl;
    return kError;
  }
  return kSuccess;
}

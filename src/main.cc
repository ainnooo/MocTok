#include <linux/if_link.h>
#include <net/if.h>

#include <string>

#include <cmdline.h>

#include "base/define/define.h"
#include "moctok.h"

namespace {
std::string kDefaultYamlFilepath = "access.yaml";
std::string kDefaultBpfFilepath = "xdp-generated.o";
std::string kDefaultIfname = "eth1";
std::string kDefaultSec = "xdp_generated";
}  // namespace

int main(int argc, char** argv) {
  // Make a rule of cmdline parser.
  cmdline::parser parser;
  parser.add<std::string>("gen", 'g',
                          "Generate XDP program. Specify yaml file's path.",
                          false, kDefaultYamlFilepath);
  parser.add("attach", 'a', "Attach XDP program.");
  parser.add("detach", 'd', "Detach XDP program.");
  parser.add("stats", 's', "Display filtering stats.");
  parser.add<std::string>("filepath", 'f', "Specify BPF file's path.", false,
                          kDefaultBpfFilepath);
  parser.add<std::string>("interface", 'i', "Specify interface.", false,
                          kDefaultIfname);
  parser.add<std::string>("sec", '\0',
                          "[Advanced option] Specify program section.", false,
                          kDefaultSec);
  parser.add("help", 'h', "Print usage.");

  // TODO: Limit option combinations.
  if (!parser.parse(argc, argv) || parser.exist("help")) {
    std::cerr << parser.error_full() << parser.usage();
    return 0;
  }

  struct config cfg = {
      .xdp_flags = XDP_FLAGS_UPDATE_IF_NOEXIST,
      // TODO: Remove ifindex from cfg.
      .ifindex = if_nametoindex(kDefaultIfname.c_str()),
      .ifname = parser.exist("interface") ? parser.get<std::string>("interface")
                                          : kDefaultIfname,
      .bpf_filepath = parser.exist("filepath")
                          ? parser.get<std::string>("filepath")
                          : kDefaultBpfFilepath,
      .progsec =
          parser.exist("sec") ? parser.get<std::string>("sec") : kDefaultSec,
      .yaml_filepath =
          parser.exist("gen") ? parser.get<std::string>("gen") : "",
  };

  if (parser.exist("attach")) {
    cfg.mode = Mode::Attach;
  } else if (parser.exist("detach")) {
    cfg.mode = Mode::Detach;
  } else if (parser.exist("gen")) {
    cfg.mode = Mode::Generate;
  } else if (parser.exist("stats")) {
    cfg.mode = Mode::Stats;
  }

  MocTok moctok(cfg);
  return 0;
}

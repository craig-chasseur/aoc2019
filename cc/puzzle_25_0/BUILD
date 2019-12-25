load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        "@com_google_absl//absl/strings",
        "//cc/util:check",
        "//cc/util:intcode",
    ],
)

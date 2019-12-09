package(
    default_visibility = ["//visibility:public"],
)

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "check",
    hdrs = ["check.h"],
    deps = [
        "@com_google_absl//absl/base",
    ],
)

cc_library(
    name = "intcode",
    hdrs = ["intcode.h"],
    srcs = ["intcode.cc"],
    deps = [
        "@com_google_absl//absl/strings",
        ":check",
    ],
)

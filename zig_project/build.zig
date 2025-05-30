const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "zig_project",
        .root_source_file = b.path("main.zig"),
        .target = target,
        .optimize = optimize,
    });

    const zini_dep = b.dependency("zini", .{});
    const zig_xml_dep = b.dependency("zig_xml", .{});

    exe.root_module.addImport("zini", zini_dep.module("zini"));
    exe.root_module.addImport("zig-xml", zig_xml_dep.module("xml"));

    b.installArtifact(exe);
}

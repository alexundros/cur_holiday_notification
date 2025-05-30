const std = @import("std");
const fs = std.fs;
const io = std.io;
const mem = std.mem;
const heap = std.heap;
const Allocator = mem.Allocator;

const zini = @import("zini");
const xml = @import("zig-xml");

pub fn main() !void {
    const gpa = heap.page_allocator;
    const stdout = io.getStdOut().writer();
    const args = try std.process.argsAlloc(gpa);
    defer std.process.argsFree(gpa, args);

    const proc_start = std.time.nanoTimestamp();

    var app_path_buf: [std.fs.max_path_bytes]u8 = undefined;
    const app_path = try fs.selfExePath(&app_path_buf);
    const appdir = fs.path.dirname(app_path) orelse ".";
    const workdir = try fs.cwd().realpathAlloc(gpa, ".");
    const delta1 = @as(f64, @floatFromInt(std.time.nanoTimestamp() - proc_start)) / 1e9;
    try stdout.print("* find dirs : {d:.6} сек.\n", .{delta1});

    try stdout.print("# Директория приложения: {s}\n", .{appdir});
    try stdout.print("# Рабочая директория: {s}\n", .{workdir});

    var is_error = true;
    defer {
        if (is_error) std.process.exit(1) else std.process.exit(0);
    }

    var parser = try getCfgParser(gpa, appdir, workdir);
    defer parser.deinit();

    const proc_delta = @as(f64, @floatFromInt(std.time.nanoTimestamp() - proc_start)) / 1e9;
    try stdout.print("# Обработка завершена: {d:.6} сек.\n", .{proc_delta});
    is_error = false;
}

fn getCfgFile(gpa: Allocator, appdir: []const u8, workdir: []const u8) ![]const u8 {
    const filename = "cur_holiday_notification.cfg";
    var exe_dir = try fs.openDirAbsolute(appdir, .{ .access_sub_paths = true });
    defer exe_dir.close();
    if (exe_dir.access(filename, .{})) |_| {
        return try fs.path.join(gpa, &[_][]const u8{ appdir, filename });
    } else |err| {
        if (err != error.FileNotFound) return err;
    }
    var workdir_dir = try fs.openDirAbsolute(workdir, .{ .access_sub_paths = true });
    defer workdir_dir.close();
    if (workdir_dir.access(filename, .{})) |_| {
        return try fs.path.join(gpa, &[_][]const u8{ workdir, filename });
    } else |err| {
        if (err != error.FileNotFound) return err;
    }
    return error.FileNotFound;
}

fn getCfgParser(gpa: Allocator, appdir: []const u8, workdir: []const u8) !zini.Parser {
    const stdout = io.getStdOut().writer();
    var parser = try zini.Parser.init(gpa);

    const start1 = std.time.nanoTimestamp();
    const cfg_file = try getCfgFile(gpa, appdir, workdir);
    try stdout.print("# Конфиг: {s}\n", .{cfg_file});
    const delta1 = @as(f64, @floatFromInt(std.time.nanoTimestamp() - start1)) / 1e9;
    try stdout.print("* getCfgFile: {d:.6} сек.\n", .{delta1});

    const start2 = std.time.nanoTimestamp();
    parser.loadFile(cfg_file) catch |e| {
        std.debug.print("Parse ini Error {any}\n", .{e});
        std.process.exit(1);
    };
    const delta2 = @as(f64, @floatFromInt(std.time.nanoTimestamp() - start2)) / 1e9;
    try stdout.print("* parser.loadFile: {d:.6} сек.\n", .{delta2});

    return parser;
}

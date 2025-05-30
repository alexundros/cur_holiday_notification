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

    const start = std.time.nanoTimestamp();

    var app_path_buf: [std.fs.max_path_bytes]u8 = undefined;
    const app_path = try fs.selfExePath(&app_path_buf);
    const appdir = fs.path.dirname(app_path) orelse ".";

    // realpathAlloc выполняется примерно 0.0002 сек.
    //const workdir = try fs.cwd().realpathAlloc(gpa, ".");
    const workdir = ".";

    //const d1 = @as(f64, @floatFromInt(std.time.nanoTimestamp() - proc_start)) / 1e9;
    //try stdout.print("* find dirs : {d:.6} сек.\n", .{d1});

    try stdout.print("# Директория приложения: {s}\n", .{appdir});
    try stdout.print("# Рабочая директория: {s}\n", .{workdir});

    var is_error = true;
    defer {
        if (is_error) std.process.exit(1) else std.process.exit(0);
    }

    const s1 = std.time.nanoTimestamp();

    var parser = try getCfgParser(gpa, appdir, workdir);
    defer parser.deinit();

    const d1 = @as(f64, @floatFromInt(std.time.nanoTimestamp() - s1)) / 1e9;
    try stdout.print("* getCfgParser: {d:.9} сек.\n", .{d1});

    const delta = @as(f64, @floatFromInt(std.time.nanoTimestamp() - start)) / 1e9;
    try stdout.print("# Обработка завершена: {d:.6} сек.\n", .{delta});
    is_error = false;
}

fn tryPath(gpa: Allocator, base: []const u8, filename: []const u8) !?[]u8 {
    const path = try std.fs.path.join(gpa, &[_][]const u8{ base, filename });
    if (std.fs.openFileAbsolute(path, .{})) |file| {
        file.close();
        return path;
    } else |_| {
        gpa.free(path);
        return null;
    }
}

fn getCfgParser(gpa: Allocator, appdir: []const u8, workdir: []const u8) !zini.Parser {
    const stdout = io.getStdOut().writer();
    var parser = try zini.Parser.init(gpa);

    const s1 = std.time.nanoTimestamp();

    const cfg_name = "cur_holiday_notification.cfg";
    const cfg_file = blk: {
        if (try tryPath(gpa, appdir, cfg_name)) |path| break :blk path;
        if (try tryPath(gpa, workdir, cfg_name)) |path| break :blk path;
        return error.FileNotFound;
    };

    const d1 = @as(f64, @floatFromInt(std.time.nanoTimestamp() - s1)) / 1e9;
    try stdout.print("# Конфиг: {s}\n", .{cfg_file});
    try stdout.print("* getCfgParser d1: {d:.9} сек.\n", .{d1});

    const s2 = std.time.nanoTimestamp();

    parser.loadFile(cfg_file) catch |e| {
        std.debug.print("Parse ini Error {any}\n", .{e});
        std.process.exit(1);
    };

    const d2 = @as(f64, @floatFromInt(std.time.nanoTimestamp() - s2)) / 1e9;
    try stdout.print("* parser.loadFile: {d:.9} сек.\n", .{d2});

    return parser;
}

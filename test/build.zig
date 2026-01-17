const std = @import("std");

pub fn build(b: *std.Build) void {
   const exe = b.addExecutable(.{
      .name = "main",
      .root_source_file = b.path("src/test.zig"),
      .target = b.graph.host,
   });

   const exe2 = b.addExecutable(.{
      .name = "main2",
      .target = b.graph.host,
   });

   b.installArtifact(exe);

   const run_exe = b.addRunArtifact(exe);

   const run_step = b.step("run", "Run the application");
   run_step.dependOn(&run_exe.step);

   exe2.addCSourceFile(.{ .file = b.path("src/test.c") });

   //exe2.addLibraryPath(b.path("../../../msys64/clang64/lib"));
   //exe2.addIncludePath(b.path("../../../msys64/clang64/include"));

   exe2.linkLibC();

   exe2.linkSystemLibrary("sqlite3");

   b.installArtifact(exe2);
   const run3_exe = b.addRunArtifact(exe);
   const run2_exe = b.addRunArtifact(exe2);

   const run2_step = b.step("run2", "Run the application");
   run2_step.dependOn(&run2_exe.step);
   run2_step.dependOn(&run3_exe.step);
}

const std = @import("std");
const builtin = @import("builtin");
const c = @cImport({
   @cInclude("glad/glad.h");
   @cInclude("GLFW/glfw3.h");
});

const SCR_WIDTH = 800;
const SCR_HEIGHT = 600;

export fn framebuffer_size_callback(_: ?*c.GLFWwindow, width: c_int, height: c_int) void {
   c.glViewport(0, 0, width, height);
}

pub fn processInput(window: *c.GLFWwindow) void {
   if (c.glfwGetKey(window, c.GLFW_KEY_ESCAPE) == c.GLFW_PRESS) {
      c.glfwSetWindowShouldClose(window, c.GL_TRUE);
   }
}

pub fn main() !void {
   if (c.glfwInit() == 0) {
      std.debug.print("Failed to initialize GLFW\n", .{});
      return error.GlfwInitFailed;
   }
   defer c.glfwTerminate();

   c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MAJOR, 4);
   c.glfwWindowHint(c.GLFW_CONTEXT_VERSION_MINOR, 4);
   c.glfwWindowHint(c.GLFW_OPENGL_PROFILE, c.GLFW_OPENGL_CORE_PROFILE);

   if (builtin.target.os.tag == .macos) {
      c.glfwWindowHint(c.GLFW_OPENGL_FORWARD_COMPAT, c.GL_TRUE);
   }

   const window = c.glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Enigma Engine".ptr, null, null) orelse {
      std.debug.print("Failed to create GLFW window\n", .{});
      return error.WindowCreationFailed;
   };
   defer c.glfwDestroyWindow(window);

   c.glfwMakeContextCurrent(window);
   _ = c.glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

   const get_proc_address_ptr: c.GLADloadproc = @ptrCast(&c.glfwGetProcAddress);
   if (c.gladLoadGLLoader(get_proc_address_ptr) == 0) {
         std.debug.print("Failed to initialize GLAD\n", .{});
         return error.GladInitFailed;
   }

   while (c.glfwWindowShouldClose(window) == 0) {
      processInput(window);

      c.glClearColor(0.2, 0.3, 0.3, 1.0);
      c.glClear(c.GL_COLOR_BUFFER_BIT);

      c.glfwSwapBuffers(window);
      c.glfwPollEvents();
   }

   c.glfwTerminate();
   return;
}

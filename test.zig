const std = @import("std");

const Vertex = struct {
   x: f32, y: f32, z: f32,
   r: f32, g: f32, b: f32,
};

// Create a buffer for 3 vertices (one triangle)
var triangle_data = [_]Vertex{
   .{ .x = -0.5, .y = -0.5, .z = 0.0, .r = 1.0, .g = 0.0, .b = 0.0 },
   .{ .x =  0.5, .y = -0.5, .z = 0.0, .r = 0.0, .g = 1.0, .b = 0.0 },
   .{ .x =  0.0, .y =  0.5, .z = 0.0, .r = 0.0, .g = 0.0, .b = 1.0 },
};

//var timer: f32 = 0.0;

export fn get_vertex_ptr() [*]Vertex {
   return &triangle_data;
}

export fn get_vertex_count() usize {
   return triangle_data.len;
}

//export fn update_vertices(dt: f32) void {
//   timer += dt;
//   // Rotate the triangle slightly in WASM
//   for (&triangle_data) |*v| {
//      v.x += @sin(timer) * 0.001;
//   }
//}

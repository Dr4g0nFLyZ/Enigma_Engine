#ifndef WASM_MANAGER_H
#define WASM_MANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <wasmtime.h>

class WasmManager {
public:
   WasmManager(const std::string& wasm_path) {
      config = wasm_config_new();
      engine = wasm_engine_new_with_config(config);
      store = wasmtime_store_new(engine, nullptr, nullptr);
      context = wasmtime_store_context(store);

      std::vector<uint8_t> binary = load_file(wasm_path);
      wasmtime_error_t* error = wasmtime_module_new(engine, binary.data(), binary.size(), &module);
      if (error) handle_error(error);

      error = wasmtime_instance_new(context, module, nullptr, 0, &instance, nullptr);
      if (error) handle_error(error);
   }

   ~WasmManager() {
      if (module) wasmtime_module_delete(module);
      if (store) wasmtime_store_delete(store);
      if (engine) wasm_engine_delete(engine);
   }

   // --- High Level API ---

   int32_t process_string(const std::string& input) {
      uint32_t wasm_buffer_offset = get_wasm_ptr("get_buffer_pointer");

      wasmtime_extern_t mem_item;
      if (!wasmtime_instance_export_get(context, &instance, "memory", 6, &mem_item)) {
         throw std::runtime_error("Failed to find 'memory' export");
      }

      uint8_t* memory_base = wasmtime_memory_data(context, &mem_item.of.memory);
      std::memcpy(memory_base + wasm_buffer_offset, input.c_str(), input.length());

      wasmtime_extern_t process_item;
      wasmtime_instance_export_get(context, &instance, "process_string", 14, &process_item);

      wasmtime_val_t args[1];
      args[0].kind = WASMTIME_I32;
      args[0].of.i32 = static_cast<int32_t>(input.length());

      wasmtime_val_t results[1];
      wasmtime_func_call(context, &process_item.of.func, args, 1, results, 1, nullptr);

      return results[0].of.i32;
   }

   // Helper to call a function and get an i32 (used for getting pointers/offsets)
   uint32_t get_wasm_ptr(const std::string& func_name) {
      wasmtime_extern_t func_item;
      if (!wasmtime_instance_export_get(context, &instance, func_name.c_str(), func_name.length(), &func_item)) {
         throw std::runtime_error("Export not found: " + func_name);
      }
      wasmtime_val_t results[1];
      wasmtime_error_t* error = wasmtime_func_call(context, &func_item.of.func, nullptr, 0, results, 1, nullptr);
      if (error) handle_error(error);
      return results[0].of.i32;
   }

   // Helper to get a raw pointer to a specific offset in WASM memory
   void* get_memory_ptr(uint32_t offset) {
      wasmtime_extern_t mem_item;
      if (!wasmtime_instance_export_get(context, &instance, "memory", 6, &mem_item)) {
         throw std::runtime_error("Failed to find 'memory' export");
      }
      uint8_t* memory_base = wasmtime_memory_data(context, &mem_item.of.memory);
      return static_cast<void*>(memory_base + offset);
   }

private:
   wasm_engine_t* engine;
   wasm_config_t* config;
   wasmtime_store_t* store;
   wasmtime_context_t* context;
   wasmtime_module_t* module = nullptr;
   wasmtime_instance_t instance;

   std::vector<uint8_t> load_file(const std::string& path) {
      std::ifstream file(path, std::ios::binary | std::ios::ate);
      if (!file) throw std::runtime_error("Could not open WASM file: " + path);
      std::streamsize size = file.tellg();
      file.seekg(0, std::ios::beg);
      std::vector<uint8_t> buffer(size);
      file.read((char*)buffer.data(), size);
      return buffer;
   }

   void handle_error(wasmtime_error_t* error) {
      wasm_byte_vec_t message;
      wasmtime_error_message(error, &message);
      std::string err_str(message.data, message.size);
      wasm_byte_vec_delete(&message);
      wasmtime_error_delete(error);
      throw std::runtime_error("Wasmtime Error: " + err_str);
   }
};

#endif

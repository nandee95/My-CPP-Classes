#pragma once

#include <CL/cl.h>

#include <fstream>
#include <string>
#include <vector>

#include "ScopedPtr.hpp"

namespace ocl
{
	 //https://stackoverflow.com/a/24336429/7788244
	std::string getErrorString(cl_int error)
	{
		switch (error) {
			// run-time and JIT compiler errors
		case 0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
		case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case -15: return "CL_COMPILE_PROGRAM_FAILURE";
		case -16: return "CL_LINKER_NOT_AVAILABLE";
		case -17: return "CL_LINK_PROGRAM_FAILURE";
		case -18: return "CL_DEVICE_PARTITION_FAILED";
		case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

			// compile-time errors
		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case -64: return "CL_INVALID_PROPERTY";
		case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case -66: return "CL_INVALID_COMPILER_OPTIONS";
		case -67: return "CL_INVALID_LINKER_OPTIONS";
		case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

			// extension errors
		case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
		case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
		case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
		case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
		default: return "Unknown OpenCL error";
		}
	}

	struct OCLDevice
	{
		cl_device_id id;
		bool isGpu;
		std::string name;
		cl_ulong memorySize;
	};

	std::vector<OCLDevice> GetDevices()
	{
		std::vector<OCLDevice> output;
		cl_uint platformCount;
		clGetPlatformIDs(0, nullptr, &platformCount); //get size
		ScopedPtr<cl_platform_id> platforms(new cl_platform_id[platformCount]);
		clGetPlatformIDs(platformCount, platforms, &platformCount);

		for (int p = 0; p < platformCount; p++)
		{
			cl_uint deviceCount;
			clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount); //get size
			ScopedPtr<cl_device_id> devices(new cl_device_id[deviceCount]);
			clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_ALL, deviceCount, devices, nullptr);

			for (int d = 0; d < deviceCount; d++)
			{
				//Add new device to the array
				output.push_back(OCLDevice());
				auto& t = *std::prev(output.end());
				t.id = devices[d];

				//Get name
				cl_uint charCount;
				clGetDeviceInfo(devices[d], CL_DEVICE_NAME, 0, nullptr, &charCount); //Get size first

				ScopedPtr<char> name(new char[charCount]);
				clGetDeviceInfo(devices[d], CL_DEVICE_NAME, charCount, name, nullptr);

				t.name = std::string(name, charCount-1);

				//Check if it's gpu or cpu
				cl_device_type type;
				clGetDeviceInfo(devices[d], CL_DEVICE_TYPE, sizeof(cl_device_type), &type, nullptr);

				t.isGpu = type & CL_DEVICE_TYPE_GPU;

				//Get compute units
				cl_ulong mem;
				clGetDeviceInfo(devices[d], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &mem, nullptr);

				t.memorySize = mem;
			}
		}
		return output;
	}

	class Context
	{
	public:
		cl_context ctx;
		cl_device_id deviceId;
		Context()
		{
		}

		const void Create(OCLDevice device)
		{
			cl_int ret;
			deviceId = device.id;
			ctx = clCreateContext(NULL, 1, &device.id, NULL, NULL, &ret);
			if (ret != CL_SUCCESS)	throw std::exception("Failed to create context!", 0);
		}
		~Context()
		{
			clReleaseContext(ctx);
		}
	};

	class CmdQueue
	{
	public:
		cl_command_queue queue;
		CmdQueue(Context& ctx)
		{
			cl_int ret;
			queue = clCreateCommandQueue(ctx.ctx, ctx.deviceId, 0, &ret);
			if (ret != CL_SUCCESS) throw std::exception("Failed to craete command queue");
		}

		~CmdQueue()
		{
			clFlush(queue);
			clFinish(queue);
			clReleaseCommandQueue(queue);
		}
	};

	class Buffer
	{
	public:
		cl_mem buf;
		Buffer(Context& ctx, size_t size, cl_mem_flags flags)
		{
			cl_int ret;
			buf = clCreateBuffer(ctx.ctx, flags, size, NULL, &ret);
			if (ret != CL_SUCCESS) throw std::exception(getErrorString(ret).c_str());
		}

		const void Upload(CmdQueue & queue, void* data, size_t size)
		{
			cl_int ret = clEnqueueWriteBuffer(queue.queue, buf, CL_TRUE, 0, size, data, 0, NULL, NULL);
			if (ret != CL_SUCCESS) throw std::exception(getErrorString(ret).c_str());
		}

		const void Download(CmdQueue & queue, void* data, size_t size)
		{
			cl_int ret = clEnqueueReadBuffer(queue.queue, buf, CL_TRUE, 0, size, data, 0, NULL, NULL);
			if (ret != CL_SUCCESS) throw std::exception(getErrorString(ret).c_str());
		}

		~Buffer()
		{
			clReleaseMemObject(buf);
		}
	};


	class Kernel
	{
	public:
		enum SourceType
		{
			FromFile,
			FromMemory
		};

		cl_kernel kernel;
		cl_program program;
		void Create(Context& ctx, std::string name, std::string source, SourceType type = FromMemory)
		{
			if (type == FromFile)
			{
				std::ifstream ifs(source);
				if (!ifs) throw std::exception("Failed to read kernel file");
				source.assign((std::istreambuf_iterator<char>(ifs)),
					std::istreambuf_iterator<char>());
			}

			cl_int ret;
			ScopedPtr<char> src(new char[source.size() + 1]);
			strcpy_s(src, source.size() + 1, source.c_str());
			size_t s = source.size() * sizeof(char) + 1;
			program = clCreateProgramWithSource(ctx.ctx, 1, (const char**)& src, (const size_t*)& s, &ret);
			if (ret != CL_SUCCESS) throw std::exception("Failed to create program");

			// Build the program
			ret = clBuildProgram(program, 1, &ctx.deviceId, NULL, NULL, NULL);
			if (ret != CL_BUILD_SUCCESS) {
				size_t log_size;
				clGetProgramBuildInfo(program,ctx.deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

				char* log = (char*)malloc(log_size);

				clGetProgramBuildInfo(program, ctx.deviceId, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

				printf("%s\n", log);

				throw std::exception("Failed to build program");
			}

			// Create the OpenCL kernel
			kernel = clCreateKernel(program, name.c_str(), &ret);
			if (ret != CL_BUILD_SUCCESS) throw std::exception("Failed to create kernel");
		}

		~Kernel()
		{
			clReleaseKernel(kernel);
			clReleaseProgram(program);
		}

		const void SetArg(cl_int position, Buffer & buf)
		{
			if (clSetKernelArg(kernel, position, sizeof(cl_mem), (void*)& buf.buf) != CL_SUCCESS) throw std::exception("Failed to set kernel argument");
		}

		const void Execute(CmdQueue & queue, size_t globalSize, size_t localsize)
		{
			if (clEnqueueNDRangeKernel(queue.queue, kernel, 1, NULL, &globalSize, &localsize, 0, NULL, NULL) != CL_SUCCESS) throw std::exception("Failed to execute kernel");
		}
	};
}


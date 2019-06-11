#pragma once
#include <GL/glew.h>
#include <png.h>

#include "Exception.hpp"
#include "ScopedPtr.hpp"

namespace gl
{
	class Texture
	{
	private:
		GLuint id;
	protected:
		struct ImageDetails
		{
			unsigned int width = 0;
			unsigned int height = 0;
			bool alpha = false;
			ScopedPtr<unsigned char> pixels;
		};
	public:
		Texture()
		{
			glGenTextures(1, &id);
		}

		~Texture()
		{
			glDeleteTextures(1, &id);
		}

		void Create(const GLuint width, const GLuint height, const GLenum colors, const void* pixels) const noexcept //GL_RGB GL_RGBA
		{
			Bind(0);
			SetWrapping(GL_REPEAT);
			SetFiltering(GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, colors, width, height, 0, colors, GL_UNSIGNED_BYTE, pixels);
		}

		void LoadFromFile(const std::string filename) const
		{
			size_t pos = filename.find_last_of('.');
			if(pos == -1) throw Exception(Exception::File_NotFound, "File doesn't have an extension!");
			pos++;
			const std::string extension = filename.substr(pos, filename.size() - pos);

			ImageDetails details;
			try
			{
				if (extension == "png")
					LoadPNG(filename, details);
				/*else if (extension == "jpg" || extension == "jpeg")
					LoadJPEG(filename, details);*/
				else throw Exception(Exception::File_UnknownFormat, "File format " + extension + " not supported!");
			}
			catch (Exception& e)
			{
				throw e;
			}

			Bind(0);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, details.width, details.height, 0,
				details.alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)details.pixels.ptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		void SetFiltering(const GLenum value) const noexcept //GL_LINEAR GL_NEAREST GL_NEAREST_MIPMAP_NEAREST GL_LINEAR_MIPMAP_NEAREST GL_NEAREST_MIPMAP_LINEAR GL_LINEAR_MIPMAP_LINEAR
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);
		}

		void SetWrapping(const GLenum value) const noexcept //GL_REPEAT GL_CLAMP_TO_EDGE GL_MIRRORED_REPEAT GL_CLAMP_TO_EDGE
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);
		}


		void MakeMipmap() const noexcept
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		inline void Bind(const GLuint index) const noexcept
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, id);
		}

		const GLuint& GetId() const noexcept
		{
			return id;
		}

	protected:
		static void LoadPNG(const std::string& filename, ImageDetails& details)
		{
			FILE* f;
			fopen_s(&f,filename.c_str(), "rb");
			E_THROW_IF(!f,Exception::File_NotFound, "File " + filename + " could not be found!")
			{
				png_byte header[8];
				fread(header, 1, 8, f);
				std::cout << png_sig_cmp(header, 0, 8) << std::endl;
				if (png_sig_cmp(header, 0, 8) != NULL) {
					fclose(f);
					throw Exception(Exception::File_Broken, "File header validation failed");
				}
			}
			png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (!png_ptr) {
				fclose(f);
				throw Exception(Exception::File_Broken, "Failed to read file #1");
			}
			
			png_infop info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) {
				png_destroy_read_struct(&png_ptr, (png_infopp)NULL,
					(png_infopp)NULL);
				fclose(f);
				throw Exception(Exception::File_Broken, "Failed to read file #2");
			}

			png_infop end_info = png_create_info_struct(png_ptr);
			if (!end_info) {
				png_destroy_read_struct(&png_ptr, (png_infopp)NULL,
					(png_infopp)NULL);
				fclose(f);
				throw Exception(Exception::File_Broken, "Failed to read file #3");
			}
			if (setjmp(png_jmpbuf(png_ptr))) {
				png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
				fclose(f);
				throw Exception(Exception::File_Broken, "Failed to read file #4");
			}
			png_init_io(png_ptr, f);
			png_set_sig_bytes(png_ptr, 8);
			png_read_info(png_ptr, info_ptr);
			png_uint_32 t_width, t_height;
			int bit_depth, color_type;
			png_get_IHDR(png_ptr, info_ptr, &t_width, &t_height, &bit_depth,&color_type, NULL, NULL, NULL);
			details.width = t_width;
			details.height = t_height;

			png_read_update_info(png_ptr, info_ptr);
			int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
			details.pixels=new unsigned char[row_bytes * t_height];
			if (!details.pixels.ptr) {
				png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
				fclose(f);
				throw Exception(Exception::File_Broken, "Failed to read file #5");
			}
			png_bytepp row_pointers = new png_bytep[t_height];
			if (!row_pointers) {
				png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
				delete details.pixels.ptr;
				fclose(f);
				throw Exception(Exception::File_Broken, "Failed to read file #6");
			}
			for (unsigned i = 0; i < t_height; ++i) {
				row_pointers[t_height - 1 - i] = details.pixels.ptr + i * row_bytes;
			}
			png_read_image(png_ptr, row_pointers);
			switch (png_get_color_type(png_ptr, info_ptr)) {
			case PNG_COLOR_TYPE_RGBA:
				details.alpha = true;
				break;
			case PNG_COLOR_TYPE_RGB:
				details.alpha = false;
				break;
			default:
				png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
				fclose(f);
				throw Exception(Exception::File_Broken, "Color type #" + std::to_string(png_get_color_type(png_ptr, info_ptr)) + " not supported!");
			}

			png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
			delete row_pointers;
			fclose(f);
		}
	};
}
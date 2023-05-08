#include "BinaryShaderCache.h"
#include "IGfxCapabilities.h"
#include "../ServiceLocator.h"
#include "../Base/Algorithms.h"
#include "../IO/FileSystem.h"
#include "../IO/IFileStream.h"
#include "../Base/HashFunctions.h"
#include "../../Common.h"

namespace nCine
{
	namespace
	{
		constexpr uint64_t HashSeed = 0x01000193811C9DC5;

		unsigned int bufferSize = 0;
		std::unique_ptr<uint8_t[]> bufferPtr;
	}

	BinaryShaderCache::BinaryShaderCache(const StringView& path)
		: isAvailable_(false), platformHash_(0)
	{
		if (path.empty()) {
			LOGD_X("Binary shader cache is disabled");
			return;
		}

		const IGfxCapabilities& gfxCaps = theServiceLocator().gfxCapabilities();
#if defined(WITH_OPENGLES) && !defined(DEATH_TARGET_EMSCRIPTEN) && !defined(DEATH_TARGET_UNIX)
		const bool isSupported = gfxCaps.hasExtension(IGfxCapabilities::GLExtensions::ARB_GET_PROGRAM_BINARY) ||
								 gfxCaps.hasExtension(IGfxCapabilities::GLExtensions::OES_GET_PROGRAM_BINARY);
#else
		const bool isSupported = gfxCaps.hasExtension(IGfxCapabilities::GLExtensions::ARB_GET_PROGRAM_BINARY);
#endif
		if (!isSupported) {
			LOGW_X("GL_ARB_get_program_binary extensions not supported, binary shader cache is disabled");
			return;
		}

#if defined(WITH_OPENGLES) && !defined(DEATH_TARGET_EMSCRIPTEN) && !defined(DEATH_TARGET_UNIX)
		if (gfxCaps.hasExtension(IGfxCapabilities::GLExtensions::OES_GET_PROGRAM_BINARY)) {
			_glGetProgramBinary = glGetProgramBinaryOES;
			_glProgramBinary = glProgramBinaryOES;
			_glProgramBinaryLength = GL_PROGRAM_BINARY_LENGTH_OES;
		} else
#endif
		{
			_glGetProgramBinary = glGetProgramBinary;
			_glProgramBinary = glProgramBinary;
			_glProgramBinaryLength = GL_PROGRAM_BINARY_LENGTH;
		}

		const IGfxCapabilities::GlInfoStrings& infoStrings = gfxCaps.glInfoStrings();

		// For a stable hash, the OpenGL strings need to be copied so that padding bytes can be set to zero
		char platformString[512];
		std::memset(platformString, 0, sizeof(platformString));
#if defined(DEATH_TARGET_WINDOWS)
		strncpy_s(platformString, infoStrings.renderer, sizeof(platformString));
#else
		strncpy(platformString, infoStrings.renderer, sizeof(platformString));
#endif
		platformHash_ += fasthash64(platformString, strnlen(platformString, sizeof(platformString)), HashSeed);

		std::memset(platformString, 0, sizeof(platformString));
#if defined(DEATH_TARGET_WINDOWS)
		strncpy_s(platformString, infoStrings.glVersion, sizeof(platformString));
#else
		strncpy(platformString, infoStrings.glVersion, sizeof(platformString));
#endif
		platformHash_ += fasthash64(platformString, strnlen(platformString, sizeof(platformString)), HashSeed);

		path_ = path;
		fs::CreateDirectories(path_);

		bufferSize = 64 * 1024;
		bufferPtr = std::make_unique<uint8_t[]>(bufferSize);

		const bool pathExists = fs::IsDirectory(path_);
		isAvailable_ = (isSupported && pathExists);
	}

	String BinaryShaderCache::getCachedShaderPath(const char* shaderName)
	{
		if (!isAvailable_ || shaderName == nullptr) {
			return { };
		}

		std::size_t shaderNameLength = strlen(shaderName);
		if (shaderNameLength == 0) {
			return { };
		}

		uint64_t shaderNameHash = fasthash64(shaderName, shaderNameLength, 0x01000193811C9DC5);

		char outputBuffer[48];
		formatString(outputBuffer, sizeof(outputBuffer), "%016llx%016llx.shader", shaderNameHash, platformHash_);
		return fs::JoinPath(path_, outputBuffer);
	}

	bool BinaryShaderCache::loadFromCache(const char* shaderName, uint64_t shaderVersion, GLShaderProgram* program, GLShaderProgram::Introspection introspection)
	{
		String cachePath = getCachedShaderPath(shaderName);
		if (cachePath.empty()) {
			return false;
		}

		std::unique_ptr<IFileStream> fileHandle = fs::Open(cachePath, FileAccessMode::Read);
		const long int fileSize = fileHandle->GetSize();
		if (fileSize <= 28 || fileSize > 8 * 1024 * 1024) {
			return false;
		}

		if (bufferSize < fileSize) {
			bufferSize = fileSize;
			bufferPtr = std::make_unique<uint8_t[]>(bufferSize);
		}

		fileHandle->Read(bufferPtr.get(), fileSize);
		fileHandle->Close();

		uint64_t signature = *(uint64_t*)&bufferPtr[0];
		uint64_t cachedShaderVersion = *(uint64_t*)&bufferPtr[8];

		// Shader version must be the same
		if (signature != 0x20AA8C9FF0BFBBEF || cachedShaderVersion != shaderVersion) {
			return false;
		}

		int32_t batchSize = *(int32_t*)&bufferPtr[16];
		uint32_t binaryFormat = *(uint32_t*)&bufferPtr[20];
		int32_t bufferLength = *(int32_t*)&bufferPtr[24];
		void* buffer = &bufferPtr[28];

		if (bufferLength <= 0 || bufferLength > fileSize - 28) {
			return false;
		}

		_glProgramBinary(program->glHandle(), binaryFormat, buffer, bufferLength);
		program->setBatchSize(batchSize);
		return program->finalizeAfterLinking(introspection);
	}

	bool BinaryShaderCache::saveToCache(const char* shaderName, uint64_t shaderVersion, GLShaderProgram* program)
	{
		String cachePath = getCachedShaderPath(shaderName);
		if (cachePath.empty()) {
			return false;
		}

		GLint length = 0;
		glGetProgramiv(program->glHandle(), _glProgramBinaryLength, &length);
		if (length <= 0) {
			return false;
		}

		if (bufferSize < length) {
			bufferSize = length;
			bufferPtr = std::make_unique<uint8_t[]>(bufferSize);
		} 

		length = 0;
		unsigned int binaryFormat = 0;
		_glGetProgramBinary(program->glHandle(), bufferSize, &length, &binaryFormat, bufferPtr.get());
		if (length <= 0 || length > bufferSize) {
			return false;
		}

		std::unique_ptr<IFileStream> fileHandle = fs::Open(cachePath, FileAccessMode::Write);
		if (!fileHandle->IsOpened()) {
			return false;
		}

		fileHandle->WriteValue<uint64_t>(0x20AA8C9FF0BFBBEF);
		fileHandle->WriteValue<uint64_t>(shaderVersion);
		fileHandle->WriteValue<int32_t>(program->batchSize());
		fileHandle->WriteValue<uint32_t>(binaryFormat);
		fileHandle->WriteValue<int32_t>(length);
		fileHandle->Write(bufferPtr.get(), length);

		return true;
	}

	void BinaryShaderCache::prune()
	{
		uint8_t inputBuffer[64];

		fs::Directory dir(path_);
		while (const StringView shaderPath = dir.GetNext()) {
			if (fs::GetExtension(shaderPath) != "shader"_s) {
				continue;
			}

			StringView filename = fs::GetFileNameWithoutExtension(shaderPath);
			if (filename.size() != 32) {
				fs::RemoveFile(shaderPath);
				continue;
			}

			char componentString[17];
			std::memcpy(componentString, &filename[16], 16);
			componentString[16] = '\0';

			uint64_t platformHash = strtoull(componentString, nullptr, 16);
			if (platformHash != platformHash_) {
				fs::RemoveFile(shaderPath);
			}
		}
	}

	void BinaryShaderCache::clear()
	{
		fs::Directory dir(path_);
		while (const StringView shaderPath = dir.GetNext()) {
			fs::RemoveFile(shaderPath);
		}
	}

	/*! \return True if the path is a writable directory */
	bool BinaryShaderCache::setPath(const StringView& path)
	{
		if (!fs::IsDirectory(path) || !fs::IsWritable(path)) {
			return false;
		}

		path_ = path;
		return true;
	}
}
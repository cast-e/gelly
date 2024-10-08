#include "Dataset.h"

#include <filesystem>
#include <memory>

using namespace rtfr;

void Dataset::LoadFrame(
	const uint frameIndex, const std::filesystem::path &framePath
) {
	const auto &header = LoadFrameInfo(framePath);
	frameHeaders[frameIndex] = header;
	frameMemory[frameIndex] = new byte[GetFrameSize(header)];

	LoadFrameData(header, framePath, frameMemory[frameIndex]);
}

void Dataset::LoadAllFrameFiles(const DatasetInfo &info) {
	for (uint i = 0; i < info.frameCount; i++) {
		char frameFileName[256];
		sprintf_s(frameFileName, "frame.%04d.pos", i + 1);
		const auto framePath = info.path / "FluidFrame" / frameFileName;

		LoadFrame(i, framePath);
	}
}

Dataset::Dataset(const DatasetInfo &info)
	: frameMemory(info.frameCount), frameHeaders(info.frameCount) {
	LoadAllFrameFiles(info);
}

bool Dataset::IsLoaded() const { return !frameMemory.empty(); }

void Dataset::LoadDataset(const DatasetInfo &info) {
	if (!frameMemory.empty()) {
		for (const auto &frame : frameMemory) {
			delete[] frame;
		}
	}

	frameMemory.clear();
	frameHeaders.clear();
	frameMemory.resize(info.frameCount);
	frameHeaders.resize(info.frameCount);

	LoadAllFrameFiles(info);
}

Dataset::~Dataset() {
	for (const auto &frame : frameMemory) {
		delete[] frame;
	}
}

float *Dataset::GetParticle(uint particleIndex, uint frameIndex) const {
	const uintptr_t frameOffset = particleIndex * PARTICLE_POS_SIZE;
	return reinterpret_cast<float *>(
		reinterpret_cast<char *>(frameMemory[frameIndex]) + frameOffset
	);
}

uint Dataset::GetParticleCount(uint frameIndex) const {
	return frameHeaders[frameIndex].particleCount;
}
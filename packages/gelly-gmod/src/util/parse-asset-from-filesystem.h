#ifndef PARSE_ASSET_FROM_FILESYSTEM_H
#define PARSE_ASSET_FROM_FILESYSTEM_H

#include <PHYParser.hpp>
#include <memory>
#include <stdexcept>

#include "GMFS.h"
#include "GarrysMod/Lua/SourceCompat.h"
#include "PHYToGMod.h"
#include "scene/asset-cache.h"

#undef max
#undef min

namespace gelly::gmod::helpers {
inline auto ParseAssetFromFilesystem(const char *assetPath)
	-> std::vector<AssetCache::Bone> {
	std::string assetPathStr(assetPath);
	assetPathStr += ".phy";

	if (!FileSystem::Exists(assetPathStr.c_str())) {
		throw std::runtime_error(
			"Asset '" + assetPathStr + "' does not exist."
		);
	}

	const auto file = FileSystem::Open(assetPathStr.c_str(), "rb");
	const auto size = FileSystem::Size(file);

	auto data = std::vector<std::byte>(size);
	FileSystem::Read(reinterpret_cast<uint8_t *>(data.data()), size, file);
	FileSystem::Close(file);

	const auto phy = PhyParser::Phy{data};
	// So bone indices in the ledge have proven to be unstable.
	// Also, ledges don't entirely map to solids properly and cause name-based
	// mapping to fail, so we represent solids as merged ledges and use their
	// index to index into the text section to find the proper bone name to map
	// to.

	auto bones = std::vector<AssetCache::Bone>{};
	bones.resize(phy.getTextSection().solids.size());

	for (const auto &[index, boneMetadata] : phy.getTextSection().solids) {
		bones[index].name = boneMetadata.name;
		bones[index].vertices = {};
	}

	for (int i = 0; i < phy.getSolids().size(); i++) {
		const auto solid = phy.getSolids()[i];
		std::optional<PhyParser::TextSection::Solid> metadata = std::nullopt;
		if (phy.getTextSection().solids.contains(i)) {
			metadata = std::make_optional(phy.getTextSection().solids.at(i));
		}

		if (!metadata.has_value()) {
			continue;
		}

		auto &bone = *std::find_if(
			bones.begin(),
			bones.end(),
			[&metadata](const auto &bone) {
				return bone.name == metadata->name;
			}
		);

		auto solidVertices = solid.vertices;

		for (int i = 0; i < solid.indices.size(); i += 3) {
			Vector v0;
			v0.x = solidVertices[solid.indices[i]].x;
			v0.y = solidVertices[solid.indices[i]].y;
			v0.z = solidVertices[solid.indices[i]].z;
			Vector v1;
			v1.x = solidVertices[solid.indices[i + 1]].x;
			v1.y = solidVertices[solid.indices[i + 1]].y;
			v1.z = solidVertices[solid.indices[i + 1]].z;

			Vector v2;
			v2.x = solidVertices[solid.indices[i + 2]].x;
			v2.y = solidVertices[solid.indices[i + 2]].y;
			v2.z = solidVertices[solid.indices[i + 2]].z;

			ConvertPHYPositionToGMod(v0.x, v0.y, v0.z);
			ConvertPHYPositionToGMod(v1.x, v1.y, v1.z);
			ConvertPHYPositionToGMod(v2.x, v2.y, v2.z);

			bone.vertices.push_back(v0.x);
			bone.vertices.push_back(v0.y);
			bone.vertices.push_back(v0.z);

			bone.vertices.push_back(v1.x);
			bone.vertices.push_back(v1.y);
			bone.vertices.push_back(v1.z);

			bone.vertices.push_back(v2.x);
			bone.vertices.push_back(v2.y);
			bone.vertices.push_back(v2.z);
		}
	}

	return std::move(bones);
}
}  // namespace gelly::gmod::helpers

#endif	// PARSE_ASSET_FROM_FILESYSTEM_H

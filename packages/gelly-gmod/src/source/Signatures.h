#ifndef SIGNATURES_H
#define SIGNATURES_H

namespace sigs {
/**
 * \brief GMod version when the signatures were generated.
 */
	constexpr int VERSION = 231219;
	constexpr const char* const CMaterialSystem_GetLocalCubemap = "40 53 48 83 ec 20 48 8b d9 48 8d 0d 44 3b 0c 00 ff 15 da c4 08 00 48 8b c8 48 85 c0 75 07 48 8d 8b f0 24 00 00";
	constexpr const char* const CShaderAPIDX8_GetD3DTexture = "40 53 48 83 ec 30 44 0f be 89 48 2d 00 00 48 8b d9 45 84 c9 78 44 48 8b 89 40 2d 00 00 48 85 c9 75 05 45 33 c0 eb 18 80 79 1a 01 75 06 4c 8b 41 58 eb 0c";
	constexpr const char* const CTexture_GetTextureHandle = "48 89 5c 24 08 48 89 74 24 10 57 48 83 ec 20 33 ff 8b da 48 8b f1 85 d2";
}

#endif //SIGNATURES_H

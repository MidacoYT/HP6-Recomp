#pragma once

#include "presentation/LanguageSelectAssets.h"

namespace hp6::presentation {

// Port of the non-weighted 0x575b70 -> 0x5764a0 path used by the Rotate
// channels. Supports constant and cycle boundaries (modes 0 and 2) and the
// serialized step flag. Other native curve modes require separate ports.
float sample_sas_nonweighted_curve(const SasAnimationChannelAsset& channel,
                                   float time);

} // namespace hp6::presentation

/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace android::afutils {

// Private system_server -> AudioFlinger control. Never forwarded to the vendor HAL.
inline constexpr char kMusicHapticsParameter[] = "lineage_music_haptics";

struct MusicHapticsConfig {
    int sessionId = 0;
    int uid = -1;
    int intensity = 2;

    bool matches(int session, int clientUid) const {
        return sessionId > 0 && sessionId == session && uid == clientUid;
    }

    float gain() const {
        return intensity == 1 ? 0.35f : intensity == 2 ? 0.65f : 1.0f;
    }

    bool operator==(const MusicHapticsConfig& other) const {
        return sessionId == other.sessionId && uid == other.uid && intensity == other.intensity;
    }

    std::string toString() const {
        return std::to_string(sessionId) + "," + std::to_string(uid) + ","
                + std::to_string(intensity);
    }

    static std::optional<MusicHapticsConfig> parse(std::string_view value) {
        MusicHapticsConfig config;
        int* fields[] = {&config.sessionId, &config.uid, &config.intensity};
        for (int i = 0; i < 3; ++i) {
            const auto separator = value.find(',');
            if ((i < 2) != (separator != std::string_view::npos)) return std::nullopt;
            const auto field = value.substr(0, separator);
            if (field.empty()) return std::nullopt;
            const auto result = std::from_chars(field.data(), field.data() + field.size(), *fields[i]);
            if (result.ec != std::errc{} || result.ptr != field.data() + field.size()) {
                return std::nullopt;
            }
            if (i < 2) value.remove_prefix(separator + 1);
        }
        // Exactly one disabled representation makes recovery and comparison deterministic.
        if (config.sessionId == 0) {
            if (config.uid != -1 || config.intensity != 2) return std::nullopt;
        } else if (config.sessionId < 0 || config.uid < 0
                || config.intensity < 1 || config.intensity > 3) {
            return std::nullopt;
        }
        return config;
    }
};

// Short control-path locks only: never used from the audio processing callback.
MusicHapticsConfig getMusicHapticsConfig();
MusicHapticsConfig replaceMusicHapticsConfig(const MusicHapticsConfig& config);

} // namespace android::afutils

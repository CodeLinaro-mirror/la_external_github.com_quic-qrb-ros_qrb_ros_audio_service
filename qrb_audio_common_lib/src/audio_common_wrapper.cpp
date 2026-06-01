// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "qrb_audio_common_lib/alsa_stream.hpp"
#include "qrb_audio_common_lib/pulse_stream.hpp"
#include "qrb_audio_common_lib/audio_stream.hpp"

#include <cerrno>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <unistd.h>

namespace qrb
{
namespace audio_common_lib
{

static AudioBackend g_backend = AudioBackend::ALSA;

static bool is_pulseaudio_available()
{
  const char * runtime_dir = getenv("XDG_RUNTIME_DIR");
  bool result = false;

  if (runtime_dir) {
    std::string path = std::string(runtime_dir) + "/pulse/native";
    if (access(path.c_str(), F_OK) == 0) {
      result = true;
    }
  }

  if (result == false) {
    uid_t uid = getuid();
    std::string path = "/run/user/" + std::to_string(uid) + "/pulse/native";
    if (access(path.c_str(), F_OK) == 0)
      result = true;
  }

  if (result == false)
    if (access("/run/pulse/native", F_OK) == 0)
      result = true;

exit:
  return result;
}

AudioBackend detect_audio_backend()
{
  snd_pcm_t * pcm = nullptr;
  int err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
  if (err >= 0) {
    snd_pcm_close(pcm);
    g_backend = AudioBackend::ALSA;
    LOGI("Audio backend: ALSA");
    goto exit;
  }
  LOGI("ALSA unavailable (%s), checking PulseAudio...", snd_strerror(err));
  if (is_pulseaudio_available()) {
    g_backend = AudioBackend::PULSEAUDIO;
    LOGI("Audio backend: PulseAudio");
    goto exit;
  }
  LOGE("No audio backend detected");
  g_backend = AudioBackend::INVALID;

exit:
  return g_backend;
}

uint32_t audio_stream_open(const AudioStreamInfo & stream_info,
    stream_event_callback_func event_callback)
{
  if (g_backend == AudioBackend::ALSA) {
    return AlsaCommonStream::audio_stream_open(stream_info, event_callback);
  } else if (g_backend == AudioBackend::PULSEAUDIO) {
    return PulseCommonStream::audio_stream_open(stream_info, event_callback);
  } else {
    throw std::runtime_error("No supported backend is available");
  }
}

int audio_stream_start(uint32_t stream_handle)
{
  if (stream_handle == 0) {
    LOGE("audio_stream_start: Invalid stream_handle");
    return -EIO;
  }
  IAudioStream * stream = IAudioStream::get_stream(stream_handle);
  if (!stream) {
    LOGE("audio_stream_start: Invalid stream");
    return -EIO;
  }
  return stream->start_stream();
}

int audio_stream_mute(uint32_t stream_handle, bool mute)
{
  if (stream_handle == 0) {
    LOGE("audio_stream_mute: Invalid stream_handle");
    return -EIO;
  }
  IAudioStream * stream = IAudioStream::get_stream(stream_handle);
  if (!stream) {
    LOGE("audio_stream_mute: Invalid stream");
    return -EIO;
  }
  return stream->mute_stream(mute);
}

int audio_stream_stop(uint32_t stream_handle)
{
  if (stream_handle == 0) {
    LOGE("audio_stream_stop: Invalid stream_handle");
    return -EIO;
  }
  IAudioStream * stream = IAudioStream::get_stream(stream_handle);
  if (!stream) {
    LOGE("audio_stream_stop: Invalid stream");
    return -EIO;
  }
  return stream->stop_stream();
}

int audio_stream_close(uint32_t stream_handle)
{
  if (stream_handle == 0) {
    LOGE("audio_stream_close: Invalid stream_handle");
    return -EIO;
  }
  IAudioStream * stream = IAudioStream::get_stream(stream_handle);
  if (!stream) {
    LOGE("audio_stream_close: Invalid stream");
    return -EIO;
  }
  int ret = stream->close_stream();
  delete stream;  // ~IAudioStream() calls unregister_stream()
  return ret;
}

int audio_stream_write(uint32_t stream_handle, const void * buf, size_t length)
{
  if (stream_handle == 0) {
    LOGE("audio_stream_write: Invalid stream_handle");
    return -EIO;
  }
  IAudioStream * stream = IAudioStream::get_stream(stream_handle);
  if (!stream) {
    LOGE("audio_stream_write: Invalid stream");
    return -EIO;
  }
  return stream->write_data(buf, length);
}

}  // namespace audio_common_lib
}  // namespace qrb

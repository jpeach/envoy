#pragma once

#include "envoy/data/tap/v2alpha/common.pb.h"
#include "envoy/data/tap/v2alpha/http.pb.h"
#include "envoy/http/header_map.h"
#include "envoy/service/tap/v2alpha/common.pb.h"

#include "common/common/logger.h"

#include "extensions/common/tap/tap_config_base.h"
#include "extensions/filters/http/tap/tap_config.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace TapFilter {

class HttpTapConfigImpl : public Extensions::Common::Tap::TapConfigBaseImpl,
                          public HttpTapConfig,
                          public std::enable_shared_from_this<HttpTapConfigImpl> {
public:
  HttpTapConfigImpl(envoy::service::tap::v2alpha::TapConfig&& proto_config,
                    Extensions::Common::Tap::Sink* admin_streamer);

  // TapFilter::HttpTapConfig
  HttpPerRequestTapperPtr createPerRequestTapper(uint64_t stream_id) override;
};

class HttpPerRequestTapperImpl : public HttpPerRequestTapper, Logger::Loggable<Logger::Id::tap> {
public:
  HttpPerRequestTapperImpl(HttpTapConfigSharedPtr config, uint64_t stream_id)
      : config_(std::move(config)), stream_id_(stream_id),
        sink_handle_(config_->createPerTapSinkHandleManager(stream_id)),
        statuses_(config_->createMatchStatusVector()) {
    config_->rootMatcher().onNewStream(statuses_);
  }

  // TapFilter::HttpPerRequestTapper
  void onRequestHeaders(const Http::HeaderMap& headers) override;
  void onRequestBody(const Buffer::Instance& data) override;
  void onRequestTrailers(const Http::HeaderMap& headers) override;
  void onResponseHeaders(const Http::HeaderMap& headers) override;
  void onResponseBody(const Buffer::Instance& data) override;
  void onResponseTrailers(const Http::HeaderMap& headers) override;
  bool onDestroyLog() override;

private:
  using MutableBodyChunk =
      envoy::data::tap::v2alpha::Body* (envoy::data::tap::v2alpha::HttpStreamedTraceSegment::*)();
  using MutableMessage = envoy::data::tap::v2alpha::HttpBufferedTrace::Message* (
      envoy::data::tap::v2alpha::HttpBufferedTrace::*)();

  void onBody(const Buffer::Instance& data,
              Extensions::Common::Tap::TraceWrapperPtr& buffered_streamed_body,
              uint32_t max_buffered_bytes, MutableBodyChunk mutable_body_chunk,
              MutableMessage mutable_message);

  void makeBufferedFullTraceIfNeeded() {
    if (buffered_full_trace_ == nullptr) {
      buffered_full_trace_ = Extensions::Common::Tap::makeTraceWrapper();
    }
  }

  Extensions::Common::Tap::TraceWrapperPtr makeTraceSegment() {
    Extensions::Common::Tap::TraceWrapperPtr segment = Extensions::Common::Tap::makeTraceWrapper();
    segment->mutable_http_streamed_trace_segment()->set_trace_id(stream_id_);
    return segment;
  }

  void streamRequestHeaders();
  void streamBufferedRequestBody();
  void streamRequestTrailers();
  void streamResponseHeaders();
  void streamBufferedResponseBody();

  HttpTapConfigSharedPtr config_;
  const uint64_t stream_id_;
  Extensions::Common::Tap::PerTapSinkHandleManagerPtr sink_handle_;
  Extensions::Common::Tap::Matcher::MatchStatusVector statuses_;
  bool started_streaming_trace_{};
  const Http::HeaderMap* request_headers_{};
  const Http::HeaderMap* request_trailers_{};
  const Http::HeaderMap* response_headers_{};
  const Http::HeaderMap* response_trailers_{};
  // Must be a shared_ptr because of submitTrace().
  Extensions::Common::Tap::TraceWrapperPtr buffered_streamed_request_body_;
  Extensions::Common::Tap::TraceWrapperPtr buffered_streamed_response_body_;
  Extensions::Common::Tap::TraceWrapperPtr buffered_full_trace_;
};

} // namespace TapFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy

#pragma once

#include <memory>
#include <string>

#include "envoy/common/pure.h"

namespace Envoy {
namespace Ssl {

class TlsCertificateConfig {
public:
  virtual ~TlsCertificateConfig() = default;

  /**
   * @return a string of certificate chain.
   */
  virtual const std::string& certificateChain() const PURE;

  /**
   * @return path of the certificate chain used to identify the local side or "<inline>" if the
   * certificate chain was inlined.
   */
  virtual const std::string& certificateChainPath() const PURE;

  /**
   * @return a string of private key.
   */
  virtual const std::string& privateKey() const PURE;

  /**
   * @return path of the private key used to identify the local side or "<inline>" if the private
   * key was inlined.
   */
  virtual const std::string& privateKeyPath() const PURE;

  /**
   * @return a string of password.
   */
  virtual const std::string& password() const PURE;

  /**
   * @return path of the password file to be used to decrypt the private key or "<inline>" if the
   * password was inlined.
   */
  virtual const std::string& passwordPath() const PURE;
};

using TlsCertificateConfigPtr = std::unique_ptr<TlsCertificateConfig>;

} // namespace Ssl
} // namespace Envoy

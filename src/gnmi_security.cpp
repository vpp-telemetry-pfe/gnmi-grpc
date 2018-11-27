/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include "gnmi_security.h"

/** GetServerCredentials - Get Server Credentials according to requested type.
 * @param type Type of credentials : TLS or Insecure
 * @return
 */
std::shared_ptr<ServerCredentials> GetServerCredentials(
    const grpc::string& type) override {
  if (type == InsecureCredentialsType) {
    return InsecureServerCredentials();
  } else if (type == TlsCredentialsType) {
    SslServerCredentialsOptions ssl_opts;

    /*
     * PEM encoding of the server private key.
     * PEM encoding of the server certificate chain.
     */
    SslServerCredentialsOptions::PemKeyCertPair pkcp = {test_server1_key,
    test_server1_cert};
    ssl_opts.pem_root_certs = "";
    ssl_opts.pem_key_cert_pairs.push_back(pkcp);

    return SslServerCredentials(ssl_opts);
  } else {

  }
}

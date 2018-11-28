/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <fstream>

#include "gnmi_security.h"

/* GetFileContent - Get an entier File content
 * @param Path to the file
 * @return String containing the entire File.
 */
std::string GetFileContent(std::string path)
{
  std::ifstream ifs(path);
  if (!ifs) {
    std::cerr << "File" << path << " not found" << std::endl;
    exit(1);
  }

  std::string content((std::istreambuf_iterator<char>(ifs)),
      (std::istreambuf_iterator<char>()));

  ifs.close();

  return content;
}

/** GetServerCredentials - Get Server Credentials according to requested type.
 * @param type Type of credentials : TLS or Insecure
 * @return
 */
std::shared_ptr<ServerCredentials> GetServerCredentials(
    const grpc::string& type)
{
  if (type == InsecureCredentialsType) {
    return grpc::InsecureServerCredentials();
  } else if (type == TlsCredentialsType) {
    SslServerCredentialsOptions ssl_opts;

    SslServerCredentialsOptions::PemKeyCertPair pkcp = {
      GetFileContent(encrypt.private_key),
      GetFileContent(encrypt.chain_certs)
    };

    ssl_opts.pem_root_certs = "";
    ssl_opts.pem_key_cert_pairs.push_back(pkcp);

    return SslServerCredentials(ssl_opts);
  } else {
    //TODO
    return grpc::InsecureServerCredentials();
  }
}

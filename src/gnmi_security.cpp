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

std::shared_ptr<ServerCredentials> TlsEncrypt::GetServerCredentials()
{
  SslServerCredentialsOptions ssl_opts;

  SslServerCredentialsOptions::PemKeyCertPair pkcp = {
    GetFileContent(private_key_path),
    GetFileContent(chain_certs_path)
  };

  ssl_opts.pem_root_certs = "";
  ssl_opts.pem_key_cert_pairs.push_back(pkcp);

  return SslServerCredentials(ssl_opts);
}

std::shared_ptr<ServerCredentials> InsecureEncrypt::GetServerCredentials()
{
  return grpc::InsecureServerCredentials();
}

void ServerSecurityContext::setInsecureEncryptType()
{
    type_ = new InsecureEncrypt();
}

void ServerSecurityContext::setTlsEncryptType(std::string cert, std::string key)
{
  type_ = new TlsEncrypt(cert, key);
}

std::shared_ptr<ServerCredentials> ServerSecurityContext::Credentials()
{
  return type_->GetServerCredentials();
}

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
  std::shared_ptr<UserPassAuthProcessor> processor =
    std::shared_ptr<UserPassAuthProcessor>(new UserPassAuthProcessor());
  std::shared_ptr<grpc::ServerCredentials> creds;
  SslServerCredentialsOptions
    ssl_opts(GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY);

  SslServerCredentialsOptions::PemKeyCertPair pkcp = {
    GetFileContent(private_key_path),
    GetFileContent(chain_certs_path)
  };

  ssl_opts.pem_root_certs = "";
  ssl_opts.pem_key_cert_pairs.push_back(pkcp);

  creds = grpc::SslServerCredentials(ssl_opts);
  creds->SetAuthMetadataProcessor(processor);

  return creds;
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

/* Implement a metadataProcessor for username/password authentication */
Status UserPassAuthProcessor::Process(const InputMetadata& auth_metadata,
                                      grpc::AuthContext* context,
                                      OutputMetadata* consumed_auth_metadata,
                                      OutputMetadata* response_metadata)
{
  /*
   * context is read/write: it contains the properties of the channel peer and
   * it is the job of the Process method to augment it with properties derived
   * from the passed-in auth_metadata.
   */
  auto user_kv = auth_metadata.find("username");
  if (user_kv == auth_metadata.end()) {
    std::cerr << "Invalid Username/Password" << std::endl;
    return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                        "No username field");
  }

  auto pass_kv = auth_metadata.find("password");
  if (pass_kv == auth_metadata.end()) {
    std::cerr << "Invalid Username/Password" << std::endl;
    return grpc::Status(grpc::StatusCode::UNAUTHENTICATED,
                        "No password field");
  }

  //TODO test if username and password are good

  std::cout << "Processor" << '\n'
            << '\t' << user_kv->first << '\t' << user_kv->second << '\n'
            << '\t' << pass_kv->first << '\t' << pass_kv->second
            << std::endl;

  //consumed_auth_metadata->insert(std::make_pair("username", "lol"));
  //consumed_auth_metadata->insert(std::make_pair("password", "lol"));

  return grpc::Status::OK;
}

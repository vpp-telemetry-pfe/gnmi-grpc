
/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpcpp/security/server_credentials.h>

using grpc::ServerCredentials;
using grpc::SslServerCredentialsOptions;

struct auth_t {
  std::string username;
  std::string password;
};

extern struct auth_t auth;

/*
 * Implement Strategy Design Pattern
 * ServerSecurityContext -
 * TypeEncrypt -
 *  TlsEncrypt -
 *  InsecureEncrypt -
 */

class TypeEncrypt;

/* Provide necessary set of functions for encryption */
class ServerSecurityContext {
  private:
    TypeEncrypt *type_;

  public:
    void setInsecureEncryptType();
    void setTlsEncryptType(std::string cert, std::string key);
    std::shared_ptr<ServerCredentials> Credentials();
};

/* Set an encryption type */
class TypeEncrypt {
  public:
    virtual std::shared_ptr<ServerCredentials> GetServerCredentials() = 0;
};

/* TLS Credentials */
class TlsEncrypt : public TypeEncrypt {
  private:
    std::string chain_certs_path; // PEM path for server private key
    std::string private_key_path; // PEM server certificate chain

  public:
    TlsEncrypt() {};
    TlsEncrypt(std::string certs, std::string key)
      : chain_certs_path(certs), private_key_path(key) {};
    std::shared_ptr<ServerCredentials> GetServerCredentials() override;
    void setCredentialsPath();
};

/* Insecure Credentials */
class InsecureEncrypt : public TypeEncrypt {
  public:
    std::shared_ptr<ServerCredentials> GetServerCredentials() override;
};

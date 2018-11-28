
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
 * Abstract class.
 */
class ServerEncrypt {
  public:
    virtual std::shared_ptr<ServerCredentials> GetServerCredentials() = 0;
};

/* TLS Credentials */
class TlsEncrypt : public ServerEncrypt {
  private:
    std::string private_key_path; // PEM server certificate chain
    std::string chain_certs_path; // PEM path for server private key

  public:
    void setPrivateKeyP(std::string path) { private_key_path = path;}
    void setChainCertsP(std::string path) { chain_certs_path = path;}
    std::string getPrivateKeyP() { return private_key_path;}
    std::string getChainCertsP() { return chain_certs_path;}
    std::shared_ptr<ServerCredentials> GetServerCredentials() override;
};

/* Insecure Credentials */
class InsecureEncrypt : public ServerEncrypt {
  public:
    std::shared_ptr<ServerCredentials> GetServerCredentials() override;
};

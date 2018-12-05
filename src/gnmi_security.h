
/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpcpp/security/server_credentials.h>
#include <grpcpp/security/auth_metadata_processor.h>

using grpc::ServerCredentials;
using grpc::SslServerCredentialsOptions;
using grpc::Status;

/******************
 * Authentication *
 ******************/

/* User/Password Authentication */
class UserPassAuthProcessor : public grpc::AuthMetadataProcessor {
  public:
  Status Process(const InputMetadata& auth_metadata,
                 grpc::AuthContext* context,
                 OutputMetadata* consumed_auth_metadata,
                 OutputMetadata* response_metadata) override;
  UserPassAuthProcessor() {};
  UserPassAuthProcessor(std::string user, std::string pass)
    : username(user), password(pass) {};
  void SetPassword(std::string pass) {password = pass;};
  void SetUsername(std::string user) {password = user;};

  private:
    std::string username;
    std::string password;
};

/**************
 * Encryption *
 **************/

/* Strategy Design Pattern */

/* Set an encryption type */
class TypeEncrypt {
  public:
    virtual ~TypeEncrypt() {};
    virtual std::shared_ptr<ServerCredentials> GetServerCredentials() = 0;
};

/* TLS Credentials */
class TlsEncrypt final: public TypeEncrypt {
  private:
    std::string chain_certs_path; // PEM path for server private key
    std::string private_key_path; // PEM server certificate chain

  public:
    TlsEncrypt() {};
    TlsEncrypt(std::string certs, std::string key)
      : chain_certs_path(certs), private_key_path(key) {};
    std::shared_ptr<ServerCredentials> GetServerCredentials() override;
};

/* Insecure Credentials */
class InsecureEncrypt final: public TypeEncrypt {
  public:
    std::shared_ptr<ServerCredentials> GetServerCredentials() override;
};

/* Provide necessary set of functions for encryption */
class ServerSecurityContext {
  private:
    TypeEncrypt *type_;
    UserPassAuthProcessor *processor_;

  public:
    ServerSecurityContext()
    {type_ = NULL; processor_ = new UserPassAuthProcessor();};
    ServerSecurityContext(std::string user, std::string pass)
    {type_ = NULL; processor_ = new UserPassAuthProcessor(user, pass);};
    ~ServerSecurityContext() {delete type_; delete processor_;};
    void SetInsecureEncryptType();
    void SetTlsEncryptType(std::string cert, std::string key);
    std::shared_ptr<ServerCredentials> Credentials();
};
